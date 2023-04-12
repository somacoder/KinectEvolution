//------------------------------------------------------------------------------
// <copyright file="DepthMapPanel.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "DepthMapPanel.h"
#include "TextureLock.h"

using namespace KinectEvolution::Xaml::Controls::Base;
using namespace KinectEvolution::Xaml::Controls::DepthMap;

using namespace Concurrency;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;

using namespace WindowsPreview::Kinect;

DepthMapPanel::DepthMapPanel()
    : Panel()
{
    critical_section::scoped_lock lock(_criticalSection);

    CreateDeviceResources();
    CreateSizeDependentResources();
}

DepthMapPanel::~DepthMapPanel()
{
    Stop();
}

void DepthMapPanel::StartRenderLoop()
{
    DirectXPanel::StartRenderLoop(); // workaround for touch events TODO:// remove
}

void DepthMapPanel::StopRenderLoop()
{
    DirectXPanel::StopRenderLoop();
}

WRK::CoordinateMapper^ DepthMapPanel::CoordinateMapper::get()
{
    return _coordinateMapper;
}

void DepthMapPanel::CoordinateMapper::set(_In_ WRK::CoordinateMapper^ value)
{
    if (value == _coordinateMapper)
    {
        return;
    }

    // unsubscribe
    if (nullptr != _coordinateMapper)
    {
        _coordinateMapper->CoordinateMappingChanged -= _mapperChangedEventToken;
    }

    _coordinateMapper = value;
    _mapperChangedEventToken = _coordinateMapper->CoordinateMappingChanged += ref new TypedEventHandler<WRK::CoordinateMapper^, CoordinateMappingChangedEventArgs^>(this, &DepthMapPanel::OnMapperChanged);

    CopyXYTableToDepthMap();

    NotifyPropertyChanged("CoordinateMapper");
}

void DepthMapPanel::DepthSource::set(_In_ WRK::DepthFrameSource^ value)
{
    if (_depthSource == value)
    {
        return;
    }

    // close the previous reader
    _depthReader = nullptr;

    // set the new source
    _depthSource = value;
    if (nullptr != _depthSource)
    {
        _depthReader = _depthSource->OpenReader();
    }

    NotifyPropertyChanged("DepthSource");
}

WRK::DepthFrameSource^ DepthMapPanel::DepthSource::get()
{
    return _depthSource;
}

void DepthMapPanel::InfraredSource::set(_In_ WRK::InfraredFrameSource^ value)
{
    if (_irSource == value)
    {
        return;
    }

    // close the previous reader
    _irReader = nullptr;

    // set the new source
    _irSource = value;
    if (nullptr != _irSource)
    {
        _irReader = _irSource->OpenReader();
    }

    NotifyPropertyChanged("InfraredSource");
}

WRK::InfraredFrameSource^ DepthMapPanel::InfraredSource::get()
{
    return _irSource;
}

void DepthMapPanel::ColorSource::set(_In_ WRK::ColorFrameSource^ value)
{
    if (_colorSource == value)
    {
        return;
    }

    // close the previous reader
    _colorReader = nullptr;

    // set the new source
    _colorSource = value;
    if (nullptr != _colorSource)
    {
        _colorReader = _colorSource->OpenReader();
    }

    NotifyPropertyChanged("ColorSource");
}

WRK::ColorFrameSource^ DepthMapPanel::ColorSource::get()
{
    return _colorSource;
}

void DepthMapPanel::NotifyPropertyChanged(Platform::String^ prop)
{
    PropertyChangedEventArgs^ args = ref new PropertyChangedEventArgs(prop);
    PropertyChanged(this, args);
}

void DepthMapPanel::Update(double elapsedTime)
{
    if (_mapperChanged)
    {
        CopyXYTableToDepthMap();
        _mapperChanged = false;
    }

    if (DEPTH_PANEL_MODE::DEPTH_RAMP == PanelMode)
    {
        if (nullptr != _depthReader)
        {
            DepthFrame^ depth = _depthReader->AcquireLatestFrame();
            if (nullptr != depth)
            {
                OnDepthFrame(depth);
            }
        }
    }
    else
    {
        if (nullptr != _colorReader)
        {
            ColorFrame^ frame = _colorReader->AcquireLatestFrame();

            if (nullptr != frame)
            {

                if (nullptr != _irReader)
                {
                    InfraredFrame^ ir = _irReader->AcquireLatestFrame();
                    if (nullptr != ir)
                    {
                        OnInfraredFrame(ir);
                    }
                }

                if (nullptr != _depthReader)
                {
                    DepthFrame^ depth = _depthReader->AcquireLatestFrame();
                    if (nullptr != depth)
                    {
                        OnDepthFrame(depth);
                    }
                }

                OnColorFrame(frame);
            }
        }
    }

}

void DepthMapPanel::Render()
{
    // Set render targets to the screen.
    BeginRender(nullptr, nullptr, nullptr);

    // bind viewport the size of the control
    D3D11_VIEWPORT backBufferView = CD3D11_VIEWPORT(0.0f, 0.0f, _renderTargetWidth, _renderTargetHeight);
    _d3dContext->RSSetViewports(1, &backBufferView);

    switch (PanelMode)
    {
    case DEPTH_PANEL_MODE::DEPTH:
        SetMode(VertexMode::SurfaceWithNormal, RampMode::None);
        Render(&_depthEffect, nullptr);
        break;
    case DEPTH_PANEL_MODE::COLOR_REGISTRATION:
        SetMode(VertexMode::SurfaceWithUV, RampMode::None);
        Render(nullptr, _colorFrame);
        break;
    case DEPTH_PANEL_MODE::DEPTH_RAMP:
        SetMode(VertexMode::SurfaceWithNormal, RampMode::Color);
        Render(nullptr, nullptr);
        break;
    case DEPTH_PANEL_MODE::IR:
        _irRenderer->Render(_d3dContext.Get());
        RenderTexture(_irRenderer->InfraredImage);
        break;
    case DEPTH_PANEL_MODE::COLOR:
    {
        // lock texture(s)
        RenderLock lock(_colorFrame);
        RenderTexture(_colorFrame);
    }
        break;
    case DEPTH_PANEL_MODE::COLOR_AND_IR:
        _irRenderer->Render(_d3dContext.Get());
        RenderTexture(_irRenderer->InfraredImage);

        //clear depth stensil for overlay
        _d3dContext->ClearDepthStencilView(_backBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        // set the region of the draw to bottom corner
        float scale = .5f;
        float width = _renderTargetWidth * scale;
        float height = _renderTargetHeight * scale;
        backBufferView = CD3D11_VIEWPORT(_renderTargetWidth - width, _renderTargetHeight - height + 67, width, height);
        _d3dContext->RSSetViewports(1, &backBufferView);

        {
            // lock texture(s)
            RenderLock lock(_colorFrame);
            RenderTextureToSize(_colorFrame, (UINT)width, (UINT)height);
        }
        break;
    }

    EndRender();
}

void DepthMapPanel::ResetDeviceResources()
{
    Panel::ResetDeviceResources();

    _uvSamplerState.ReleaseAndGetAddressOf();

    _pointEffect = nullptr;
    _depthTexture = nullptr;

    _meshEffect = nullptr;
    _mesh = nullptr;
    _xyTexture = nullptr;
    _uvTexture = nullptr;
}

void DepthMapPanel::CreateDeviceResources()
{
    Panel::CreateDeviceResources();

    // depth point
    _pointEffect = ref new DepthPointEffect();
    _pointEffect->Initialize(_d3dDevice.Get(), DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT, DEPTH_MINMM, DEPTH_MAXMM);

    _depthTexture = ref new Texture();
    _depthTexture->Initialize(_d3dDevice.Get(), DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT, DXGI_FORMAT_R16_UNORM, FALSE);

    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MaxLOD = FLT_MAX;
    DX::ThrowIfFailed(
        _d3dDevice->CreateSamplerState(&samplerDesc, &_uvSamplerState)
        );

    // depth mesh
    _mesh = ref new DepthMesh();

    _mesh->Initialize(
        _d3dDevice.Get(),
        DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT,
        2 * sizeof(float),
        6 * (DEPTH_FRAME_WIDTH - 1) * (DEPTH_FRAME_HEIGHT - 1), FALSE);

    float* pVertex = static_cast<float*>(_mesh->LockVertexBuffer(_d3dContext.Get()));
    if (nullptr == pVertex)
    {
        throw ref new Platform::Exception(E_OUTOFMEMORY);
    }

    UINT* pIndex = static_cast<UINT*>(_mesh->LockIndexBuffer(_d3dContext.Get()));
    if (nullptr == pIndex)
    {
        throw ref new Platform::Exception(E_OUTOFMEMORY);
    }

    for (UINT y = 0; y < DEPTH_FRAME_HEIGHT; y++)
    {
        for (UINT x = 0; x < DEPTH_FRAME_WIDTH; x++)
        {
            *pVertex++ = static_cast<float>(x);
            *pVertex++ = static_cast<float>(y);

            if (x > 0 && y > 0)
            {
                UINT i0 = (y - 1) * DEPTH_FRAME_WIDTH + x - 1;
                UINT i1 = i0 + 1;
                UINT i2 = i0 + DEPTH_FRAME_WIDTH + 1;
                UINT i3 = i0 + DEPTH_FRAME_WIDTH;

                *pIndex++ = i0;
                *pIndex++ = i1;
                *pIndex++ = i2;
                *pIndex++ = i0;
                *pIndex++ = i2;
                *pIndex++ = i3;
            }
        }
    }

    _mesh->UnlockIndexBuffer(_d3dContext.Get());
    _mesh->UnlockVertexBuffer(_d3dContext.Get());

    _meshEffect = ref new DepthMeshEffect();
    if (nullptr != _meshEffect)
    {
        _meshEffect->Initialize(_d3dDevice.Get(), DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT, DEPTH_MINMM, DEPTH_MAXMM);
    }

    _xyTexture = ref new Texture();
    if (nullptr != _xyTexture)
    {
        _xyTexture->Initialize(_d3dDevice.Get(), DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT, DXGI_FORMAT_R32G32_FLOAT, FALSE);

        // construct default XY Table
        UINT rowPitch = 0;
        {
            TextureLock lock(_xyTexture, _d3dContext.Get());
            float* pTable = static_cast<float*>(lock.AccessBuffer(rowPitch));
            if (nullptr != pTable)
            {
                FillInDefaultXYTable(pTable, rowPitch / sizeof(float));
            }
        }
    }

    _uvTexture = ref new Texture();
    if (nullptr != _uvTexture)
    {
        _uvTexture->Initialize(_d3dDevice.Get(), DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT, DXGI_FORMAT_R32G32_FLOAT, FALSE);
    }

    ZeroMemory(&_depthEffect, sizeof(_depthEffect));
    _depthEffect._direction = XMVector3Normalize(XMVectorSet(0.5f, 0.3f, 1.5f, 0));
    _depthEffect._ambient = XMVectorSet(0.2f, 0.2f, 0.2f, 1.0f);
    _depthEffect._diffuse = XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f);
    _depthEffect._specular = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    _depthEffect._enableLighting = TRUE;

    _colorFrame = ref new Texture();
    _colorFrame->Initialize(
        _d3dDevice.Get(),
        COLOR_FRAME_WIDTH,
        COLOR_FRAME_HEIGHT,
        DXGI_FORMAT_G8R8_G8B8_UNORM,
        FALSE);

    _irRenderer = ref new InfraredRenderer();
    _irRenderer->Initialize(_d3dDevice.Get(), _d3dContext.Get());
}

void DepthMapPanel::CreateSizeDependentResources()
{
    Panel::CreateSizeDependentResources();

    _viewMatrix = XMMatrixIdentity();

    _projectionMatrix = XMMatrixPerspectiveFovLH(DirectX::XM_PI * 60.0f / 180.0f, _renderTargetWidth / _renderTargetHeight, 0.01f, 100.0f);

    Render();
}

void DepthMapPanel::UpdateDepthTexture(_In_reads_(pixels) const UINT16* pZ, UINT pixels)
{
    if (pixels != DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT)
    {
        return;
    }

    __assume(pixels == DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT);

    UINT rowPitch = 0;
    {
        TextureLock lock(_depthTexture, _d3dContext.Get());
        void* pDepthTextureData = lock.AccessBuffer(rowPitch);
        if (nullptr != pDepthTextureData)
        {
            if (rowPitch == DEPTH_FRAME_WIDTH * sizeof(UINT16))
            {
                memcpy(pDepthTextureData, pZ, pixels * sizeof(UINT16));
            }
            else
            {
                UINT16* pDst = static_cast<UINT16*>(pDepthTextureData);
                const UINT16* pSrc = pZ;
                for (UINT y = 0; y < DEPTH_FRAME_HEIGHT; y++)
                {
                    memcpy(pDst, pSrc, DEPTH_FRAME_WIDTH * sizeof(UINT16));
                    pDst += rowPitch / sizeof(UINT16);
                    pSrc += DEPTH_FRAME_WIDTH;
                }
            }
        }
    }
}

void DepthMapPanel::FillInDefaultXYTable(_Out_writes_(pitch * DEPTH_FRAME_HEIGHT) float* pTable, UINT pitch)
{
    float tanHalfFovX = tanf(DEPTH_FRAME_HFOV / 2.0f);
    float tanHalfFovY = tanf(DEPTH_FRAME_VFOV / 2.0f);
    for (UINT y = 0; y < DEPTH_FRAME_HEIGHT; ++y)
    {
        for (UINT x = 0; x < min(pitch / 2, DEPTH_FRAME_WIDTH); ++x)
        {
#pragma warning(suppress: 26014)
            pTable[2 * x] = (x - DEPTH_FRAME_WIDTH / 2.0f + 0.5f) / (DEPTH_FRAME_WIDTH / 2.0f) * tanHalfFovX;
            pTable[2 * x + 1] = (DEPTH_FRAME_HEIGHT / 2.0f - y - 0.5f) / (DEPTH_FRAME_HEIGHT / 2.0f) * tanHalfFovY;
        }
        pTable += pitch;
    }
}

void DepthMapPanel::UpdateData(_In_reads_(pixels) const UINT16* pZ, UINT pixels)
{
    if (pixels != DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT)
    {
        return;
    }

    __assume(pixels == DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT);

    if (VertexMode::PointXYZ == _vertexMode)
    {
        // It is invalid to update only Z while using VizDepthVertexMode::PointXYZ
        // use UpdateData(pX,pY,pZ) instead
        ASSERT(FALSE);
        return;
    }

    UpdateDepthTexture(pZ, pixels);
}

void DepthMapPanel::UpdateData(_In_reads_(pixels) const float* pX, _In_reads_(pixels) const float* pY, _In_reads_(pixels) const UINT16* pZ, UINT pixels)
{
    if (pixels != DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT)
    {
        return;
    }

    if (_vertexMode != VertexMode::PointXYZ)
    {
        // It is invalid to update XYZ while not using VizDepthVertexMode::PointXYZ
        // use UpdateData(pZ) instead
        ASSERT(FALSE);
        return;
    }

    UpdateDepthTexture(pZ, pixels);

    UINT rowPitch = 0;
    {
        TextureLock lock(_depthTexture, _d3dContext.Get());
        float* pTable = static_cast<float*>(lock.AccessBuffer(rowPitch));
        if (nullptr != pTable)
        {
            for (UINT y = 0; y < DEPTH_FRAME_HEIGHT; y++)
            {
                for (UINT x = 0; x < min(rowPitch / sizeof(float) / 2, DEPTH_FRAME_WIDTH); x++)
                {
#pragma warning(suppress: 26014)
                    pTable[2 * x] = *pX / *pZ;
                    pTable[2 * x + 1] = -*pY / *pZ;
                    pX++;
                    pY++;
                    pZ++;
                }
                pTable += rowPitch / sizeof(float);
            }
            
        }
    }
}

void DepthMapPanel::UpdateXYTable(_In_reads_(2 * pixels) const float* pXYTable, UINT pixels)
{
    if (pixels != DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT)
    {
        return;
    }

    UINT rowPitch = 0;
    {
        TextureLock lock(_xyTexture, _d3dContext.Get());
        float* pTable = static_cast<float*>(lock.AccessBuffer(rowPitch));
        if (nullptr != pTable)
        {
            for (UINT y = 0; y < DEPTH_FRAME_HEIGHT; y++)
            {
                memcpy(pTable, pXYTable, DEPTH_FRAME_WIDTH * 2 * sizeof(float));
                pTable += rowPitch / sizeof(float);
                pXYTable += 2 * DEPTH_FRAME_WIDTH;
            }
        }
    }
}

void DepthMapPanel::UpdateUVTable(_In_reads_(2 * pixels) const float* pUVTable, UINT pixels)
{
    if (pixels != DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT)
    {
        return;
    }

    UINT rowPitch = 0;
    {
        TextureLock lock(_uvTexture, _d3dContext.Get());
        float* pTable = static_cast<float*>(lock.AccessBuffer(rowPitch));
        if (nullptr != pTable)
        {
            for (UINT y = 0; y < DEPTH_FRAME_HEIGHT; y++)
            {
                memcpy(pTable, pUVTable, DEPTH_FRAME_WIDTH * 2 * sizeof(float));
                pTable += rowPitch / sizeof(float);
                pUVTable += 2 * DEPTH_FRAME_WIDTH;
            }
        }
    }
}

void DepthMapPanel::Render(
    _In_opt_ const PrimitiveParameter* pBasicEffectParameter,
    _In_opt_ Texture^ pSurfaceTexture)
{
    XMMATRIX worldMatrix = XMMatrixIdentity();

    if (_vertexMode == VertexMode::PointSprite)
    {
        XMMATRIX wv = worldMatrix * GetViewMatrix();
        _meshEffect->ApplyTransformAll(_d3dContext.Get(), wv, wv); // projection matrix is applied in geometry shader
    }
    else if (_vertexMode == VertexMode::PointSpriteWithCameraRotation)
    {
        _meshEffect->ApplyTransformAll(_d3dContext.Get(), worldMatrix, worldMatrix);
    }
    else
    {
        ApplyTransformWithWorldMatrix(worldMatrix);
    }

    BOOL asSurface =
        _vertexMode == VertexMode::Surface ||
        _vertexMode == VertexMode::SurfaceWithNormal ||
        _vertexMode == VertexMode::SurfaceWithUV;

    Texture^ pTexture = (_rampMode == RampMode::None
        && (asSurface || _vertexMode == VertexMode::PointSprite || _vertexMode == VertexMode::PointSpriteWithCameraRotation))
        ? dynamic_cast<Texture^>(pSurfaceTexture) : nullptr;

    // lock texture(s)
#pragma warning(suppress: 6387)
#pragma warning(suppress: 28183)
    RenderLock texLock(pTexture);
    RenderLock colorLock(_colorFrame);
    RenderLock depthLock(_depthTexture);
    RenderLock xyLock(_xyTexture);
    RenderLock uvLock(_uvTexture);

    if (!_meshEffect->Apply(_d3dContext.Get(),
        _vertexMode, _rampMode,
        _viewMatrix, _projectionMatrix,
        pBasicEffectParameter,
        _colorFrame, _pointEffect,
        _depthTexture, _xyTexture, _uvTexture))
    {
        return;
    };

    // save old sampler state
    ID3D11SamplerState* pOldSamplerState = nullptr;

    // set the sample for the texture
    _d3dContext->PSGetSamplers(0, 1, &pOldSamplerState);

    _d3dContext->PSSetSamplers(0, 1, _uvSamplerState.GetAddressOf());

    if (_vertexMode == VertexMode::SurfaceWithUV)
    {
        _d3dContext->PSSetSamplers(0, 1, _uvSamplerState.GetAddressOf());
    }

    if (asSurface)
    {
        _mesh->RenderTriangleList(_d3dContext.Get(), TRUE);
    }
    else
    {
        _mesh->RenderPointList(_d3dContext.Get());
    }

    ID3D11ShaderResourceView* nullSRVs [] = { nullptr, nullptr, nullptr };
    _d3dContext->VSSetShaderResources(0, _countof(nullSRVs), nullSRVs);
    _d3dContext->PSSetShaderResources(0, _countof(nullSRVs), nullSRVs);

    if (_vertexMode == VertexMode::SurfaceWithUV)
    {
        // restore old sampler state
        _d3dContext->PSSetSamplers(0, 1, &pOldSamplerState);
    }

    _d3dContext->PSSetSamplers(0, 1, &pOldSamplerState);
}

void DepthMapPanel::SetMode(VertexMode vertexMode, RampMode rampMode)
{
    if (vertexMode == VertexMode::SurfaceWithUV &&  rampMode != RampMode::None)
    {
        // VizDepthVertexMode::SurfaceWithUV works only with ramp mode none
        ASSERT(FALSE);
        return;
    }

    _vertexMode = vertexMode;
    _rampMode = rampMode;
}


void DepthMapPanel::OnMapperChanged(WRK::CoordinateMapper^ sender, WRK::CoordinateMappingChangedEventArgs^ args)
{
    _mapperChanged = TRUE;
}

void DepthMapPanel::CopyXYTableToDepthMap()
{
    Platform::Array<Windows::Foundation::Point>^ xyTable
        = _coordinateMapper->GetDepthFrameToCameraSpaceTable();

    UpdateXYTable(reinterpret_cast<float*>(xyTable->Data), xyTable->Length);
}

void DepthMapPanel::OnDepthFrame(_In_ WRK::DepthFrame^ frame)
{
    if (nullptr == frame)
    {
        return;
    }

    if (_depthFrame == nullptr)
    {
        _depthFrame = ref new Platform::Array<UINT16>(DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT);
    }

    if (_uvTable == nullptr)
    {
        _uvTable = ref new Platform::Array<ColorSpacePoint>(DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT);
    }

    frame->CopyFrameDataToArray(_depthFrame);

    if (PanelMode == DEPTH_PANEL_MODE::COLOR_REGISTRATION)
    {
        _coordinateMapper->MapDepthFrameToColorSpace(_depthFrame, _uvTable);
        UpdateUVTable(reinterpret_cast<float*>(_uvTable->Data), frame->FrameDescription->LengthInPixels);
    }

    UpdateData(_depthFrame->Data, frame->FrameDescription->LengthInPixels);
}

void DepthMapPanel::OnInfraredFrame(_In_ WRK::InfraredFrame^ frame)
{
    if (nullptr == frame)
    {
        return;
    }

    UINT length = frame->FrameDescription->LengthInPixels * frame->FrameDescription->BytesPerPixel;

    IBuffer^ buffer = frame->LockImageBuffer();
    UINT16* pSrc = reinterpret_cast<UINT16*>(DX::GetPointerToPixelData(buffer));

    _irRenderer->UpdateFrameImage(_d3dContext.Get(), length, pSrc);
}

void DepthMapPanel::OnColorFrame(_In_ WRK::ColorFrame^ frame)
{
    if (nullptr == frame)
    {
        return;
    }

    IBuffer^ buffer = frame->LockRawImageBuffer();
    BYTE* pColorData = reinterpret_cast<BYTE*>(DX::GetPointerToPixelData(buffer));

    UINT rowPitch = 0;
    TextureLock lock(_colorFrame, _d3dContext.Get());
    BYTE* pOutput = static_cast<BYTE*>(lock.AccessBuffer(rowPitch));
    if (nullptr != pOutput)
    {
        memcpy(pOutput, pColorData, buffer->Length);
    }
}
