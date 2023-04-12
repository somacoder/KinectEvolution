//------------------------------------------------------------------------------
// <copyright file="InfraredPanel.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "InfraredPanel.h"
#include "DirectXHelper.h"
#include "shaders.h"
#include "TextureLock.h"

using namespace KinectEvolution::Xaml::Controls::Base;
using namespace KinectEvolution::Xaml::Controls::Infrared;

using namespace Concurrency;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;

using namespace WindowsPreview::Kinect;

InfraredPanel::InfraredPanel()
    : Panel()
{
    critical_section::scoped_lock lock(_criticalSection);

    CreateDeviceResources();
    CreateSizeDependentResources();
}

InfraredPanel::~InfraredPanel()
{
    Stop();
}

void InfraredPanel::StartRenderLoop()
{
    DirectXPanel::StartRenderLoop();
}

void InfraredPanel::StopRenderLoop()
{
    DirectXPanel::StopRenderLoop();
}

/// <summary>
/// Sets the color source for this control
/// </summary>
void InfraredPanel::InfraredSource::set(InfraredFrameSource^ value)
{
    if (_frameSource == value)
    {
        return;
    }

    // close the previous reader
    _frameReader = nullptr;

    // set the new source
    _frameSource = value;
    _frameReader = _frameSource->OpenReader();

    NotifyPropertyChanged("InfraredSource");
}

InfraredFrameSource^ InfraredPanel::InfraredSource::get()
{
    return _frameSource;
}

void InfraredPanel::NotifyPropertyChanged(Platform::String^ prop)
{
    PropertyChangedEventArgs^ args = ref new PropertyChangedEventArgs(prop);
    PropertyChanged(this, args);
}

void InfraredPanel::Update(double elapsedTime)
{
    if (nullptr != _frameReader)
    {
        InfraredFrame^ frame = _frameReader->AcquireLatestFrame();
        if (nullptr != frame)
        {
            OnInfraredFrame(frame);
        }
    }
}

void InfraredPanel::Render()
{
    if (!IsLoadingComplete())
    {
        return;
    }

    // Set render target to the texture.
    BeginRender(_rampRTV.Get(), _rampDSV.Get(), nullptr);

    // Create and set viewport.
    const D3D11_VIEWPORT txtView = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(IR_FRAME_WIDTH), static_cast<float>(IR_FRAME_HEIGHT));

    _d3dContext->RSSetViewports(1, &txtView);

    {
        // lock texture(s)
        RenderLock irRampLock(_irRampTexture);
        RenderLock irLock(_irTexture);

        // do the conversion on GPU
        RampEffect::RampParameter param
            = { XMVectorZero(), 1.0f, 0.0f, FLT_MAX, static_cast<float>(max(_irRampTexture->Width, _irRampTexture->Height)) };

        // apply ramp conversion effect
        if (!_rampEffect->Apply(_d3dContext.Get(), _irTexture->TextureSRV, _irRampTexture->TextureSRV, &param))
        {
            return;
        }

        // Set primitive topology
        _d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

        // Draw the scene
        _d3dContext->Draw(1, 0);
    }

    // reset what we did
    ID3D11Buffer* buffer = nullptr;
    _d3dContext->GSSetConstantBuffers(0, 1, &buffer);
    _d3dContext->PSSetConstantBuffers(0, 1, &buffer);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    _d3dContext->PSSetShaderResources(0, 1, &nullSRV);


    // Render texture to the back buffer for correct aspect ratio and size for control
    BeginRender(nullptr, nullptr, nullptr);

    const D3D11_VIEWPORT backBufferView = CD3D11_VIEWPORT(0.0f, 0.0f, _renderTargetWidth, _renderTargetHeight);
    _d3dContext->RSSetViewports(1, &backBufferView);

    {
        // lock texture(s)
        RenderLock lock(_irTargetFrame);

        RenderTexture(_irTargetFrame);
    }

    EndRender();

}

void InfraredPanel::OnInfraredFrame(_In_ InfraredFrame^ frame)
{
    if (nullptr == frame)
    {
        return;
    }

    UINT length = frame->FrameDescription->LengthInPixels;

    IBuffer^ buffer = frame->LockImageBuffer();
    UINT16* pSrc = reinterpret_cast<UINT16*>(DX::GetPointerToPixelData(buffer));

    UINT rowPitch = 0;
    {
        TextureLock lock(_irTexture, _d3dContext.Get());
        UINT16* pDest = static_cast<UINT16*>(lock.AccessBuffer(rowPitch));
        if (nullptr != pDest)
        {
            memcpy(pDest, pSrc, length);
        }
    }
}

void InfraredPanel::ResetDeviceResources()
{
    Panel::ResetDeviceResources();

    _rampRTV.ReleaseAndGetAddressOf();
    _rampDSV.ReleaseAndGetAddressOf();

    _rampEffect = nullptr;
    _irTargetFrame = nullptr;
    _irTexture = nullptr;
    _irRampTexture = nullptr;

    Render(); // render one frame to force resize
}

void InfraredPanel::CreateDeviceResources()
{
    Panel::CreateDeviceResources();

    _rampEffect = ref new RampEffect();
    _rampEffect->Initialize(_d3dDevice.Get());

    // if everything is loaded, then we can create the texture
    _irTexture = ref new Texture();
    _irTexture->Initialize(_d3dDevice.Get(), IR_FRAME_WIDTH, IR_FRAME_HEIGHT, DXGI_FORMAT_R16_UNORM, FALSE);

    // grey scale ramp to use for shader
    _irRampTexture = ref new Texture();
    _irRampTexture->Initialize(_d3dDevice.Get(), IR_FRAME_WIDTH, 1, DXGI_FORMAT_R8G8B8A8_UNORM, FALSE);

    UINT rowPitch = 0;
    {
        TextureLock lock(_irRampTexture, _d3dContext.Get());
        UINT16* pRamp = static_cast<UINT16*>(lock.AccessBuffer(rowPitch));
        if (nullptr != pRamp)
        {
            for (UINT i = 0; i < IR_FRAME_WIDTH; ++i)
            {
                float value = min(1, 1.0f * powf(static_cast<float>(i) / 511, 0.32f));
                UINT greyLevel = static_cast<UINT>(value * 255);
                *pRamp++ = 0xFF000000 | (greyLevel << 16) | (greyLevel << 8) | greyLevel;
            }
        }
    }

    // render target creation
    _irTargetFrame = ref new Texture();
    _irTargetFrame->Initialize(
        _d3dDevice.Get(),
        IR_FRAME_WIDTH, IR_FRAME_HEIGHT,
        DXGI_FORMAT_R8G8B8A8_UNORM, TRUE);

    DX::ThrowIfFailed(
        _d3dDevice->CreateRenderTargetView(_irTargetFrame->Texture2D, nullptr, &_rampRTV)
        );

    // Create depth/stencil buffer descriptor.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        static_cast<UINT>(IR_FRAME_WIDTH),
        static_cast<UINT>(IR_FRAME_HEIGHT),
        1,
        1,
        D3D11_BIND_DEPTH_STENCIL);

    // Allocate a 2-D surface as the depth/stencil buffer.
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(
        _d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil)
        );

    // Create depth/stencil view based on depth/stencil buffer.
    const CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc
        = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(
        _d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, &_rampDSV)
        );

}

void InfraredPanel::CreateSizeDependentResources()
{
    Panel::CreateSizeDependentResources();

    Render();
}
