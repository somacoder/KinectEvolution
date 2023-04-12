//------------------------------------------------------------------------------
// <copyright file="DepthMeshEffect.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "DepthMeshEffect.h"
#include "shaders.h"

using namespace KinectEvolution::Xaml::Controls::DepthMap;

struct VSConstants
{
    // CONST 0      w, h, min_z(in mm), max_z(in mm)
    float _width;
    float _height;
    float _minZmm;
    float _maxZmm;

    // CONST 1      w, h, vertex mode
    float _textureWidth;
    float _textureHeight;
    float _vertexMode;
    float _notused;
};

struct PSConstants
{
    // CONST 0
    XMVECTOR _color; // rgba of point color when no ramp texture

    // CONST 1      1/w, 1/h, min_z(in mm), max_z(in mm)
    float _invW;
    float _invH;
    float _minZmm;
    float _maxZmm;

    // CONST 2
    float _vertexMode;
    float _rampMode;
    float _useSurfaceTexture;
    float _pad;

    // CONST 3      enabled/textured
    float _enableLight;
    float _enableTexture;
    float _specularPower;
    float _ambientOnlyLighting;

    // CONST 4-6    ambient/diffuse/specular color
    XMVECTOR _ambient;
    XMVECTOR _diffuse;
    XMVECTOR _specular;

    // CONST 7      light direction
    XMVECTOR _lightDir;

    // CONST 8-9 enable color multiplier / color multiplier
    float _enableMultiplyColor;
    float _pad2;
    float _pad3;
    float _pad4;
    XMVECTOR _multiplyColor;
};

struct GSConstants
{
    // CONST 0 - 4
    float _proj[16]; // projection XMMATRIX

    // CONST 5
    float _pixelSize; // pixel height in mm
    float _pad1[3];
};

DepthMeshEffect::DepthMeshEffect()
    : Effect()
    , _geometryShader(nullptr)
    , _vsConstantBuffer(nullptr)
    , _gsConstantBuffer(nullptr)
    , _psConstantBuffer(nullptr)
    , _vsSampler(nullptr)
    , _width(0)
    , _height(0)
    , _minZmm(0)
    , _maxZmm(0)
    , _pixelSizemm(0)
{
}

void DepthMeshEffect::Initialize(_In_ ID3D11Device1* const pD3DDevice, UINT width, UINT height, float minZmm, float maxZmm)
{
    // Asynchronously load vertex shader and create input layout.
    auto loadVSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\DepthMeshVS.cso");
    auto createVSTask = loadVSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        CONST D3D11_INPUT_ELEMENT_DESC DepthPointVertexElems[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        Effect::InitializeVS(pD3DDevice, &fileData[0], fileData.size(), DepthPointVertexElems, _countof(DepthPointVertexElems));

    });

    // Asynchronously load pixel shader and create constant buffer.
    auto loadPSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\DepthMeshPS.cso");
    auto createPSTask = loadPSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        Effect::InitializePS(pD3DDevice, &fileData[0], fileData.size());

    });

    // Asynchronously load pixel shader and create constant buffer.
    auto loadGSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\DepthMeshGS.cso");
    auto createGSTask = loadGSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        DX::ThrowIfFailed(
            pD3DDevice->CreateGeometryShader(
            &fileData[0],
            fileData.size(),
            nullptr,
            &_geometryShader));
    });

    auto createShadersTask = (createPSTask && createVSTask && createGSTask).then([this, pD3DDevice, width, height, minZmm, maxZmm]() {

        // create VS const buffer
        D3D11_BUFFER_DESC CBDesc = { 0 };
        CBDesc.ByteWidth = sizeof(VSConstants);
        CBDesc.Usage = D3D11_USAGE_DYNAMIC;
        CBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        CBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        DX::ThrowIfFailed(
            pD3DDevice->CreateBuffer(&CBDesc, nullptr, &_vsConstantBuffer)
            );

        // create PS const buffer
        CBDesc.ByteWidth = sizeof(PSConstants);
        DX::ThrowIfFailed(
            pD3DDevice->CreateBuffer(&CBDesc, nullptr, &_psConstantBuffer)
            );

        // create GS const buffer
        CBDesc.ByteWidth = sizeof(GSConstants);
        DX::ThrowIfFailed(
            pD3DDevice->CreateBuffer(&CBDesc, nullptr, &_gsConstantBuffer)
            );

        CBDesc.ByteWidth = sizeof(MatrixViewConstants);
        DX::ThrowIfFailed(
            pD3DDevice->CreateBuffer(&CBDesc, nullptr, &_cameraConstantBuffer)
            );

        // create VS sampler state
        D3D11_SAMPLER_DESC samplerDesc;
        ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.MaxLOD = FLT_MAX;
        DX::ThrowIfFailed(
            pD3DDevice->CreateSamplerState(&samplerDesc, &_vsSampler)
            );

        _width = width;
        _height = height;
        _minZmm = minZmm;
        _maxZmm = maxZmm;
        _pixelSizemm = 3;

    });

    // Once the everything is dine, we can start rendering
    createShadersTask.then([this]() {
        _loadingComplete = true;
    });
}

BOOL DepthMeshEffect::Apply(
    _In_ ID3D11DeviceContext1* const pD3DContext,
    VertexMode vertexMode,
    RampMode rampMode,
    _In_ const XMMATRIX& view,
    _In_ const XMMATRIX& proj,
    _In_opt_ const PrimitiveParameter* pEffect,
    _In_opt_ Texture^ pTexture,
    _In_opt_ DepthPointEffect^ pDepthPointEffect,
    _In_opt_ Texture^ pDepthTexture,
    _In_opt_ Texture^ pXYTexture,
    _In_opt_ Texture^ pUVTexture)
{
    if (!Effect::Apply(pD3DContext))
    {
        return FALSE;
    }

    ID3D11ShaderResourceView* vssrvs[] = { nullptr, nullptr, nullptr };
    ID3D11ShaderResourceView* pssrvs[] = { nullptr, nullptr };

    if (nullptr != pTexture)
    {
        pssrvs[0] = pTexture->TextureSRV;
    }

    if (rampMode != RampMode::None && nullptr != pDepthPointEffect)
    {
        switch (rampMode)
        {
        case RampMode::Color:
            pssrvs[0] = pDepthPointEffect->ColorRampSRV;
            break;
        case RampMode::Grey:
            pssrvs[0] = pDepthPointEffect->GreyRampSRV;
            break;
        }
    }

    if (nullptr != pDepthTexture)
    {
        pssrvs[1] = pDepthTexture->TextureSRV;
        vssrvs[0] = pDepthTexture->TextureSRV;
    }

    if (nullptr != pXYTexture)
    {
        vssrvs[1] = pXYTexture->TextureSRV;
    }

    if (nullptr != pUVTexture)
    {
        vssrvs[2] = pUVTexture->TextureSRV;
    }
    pD3DContext->VSSetShaderResources(0, _countof(vssrvs), vssrvs);
    pD3DContext->PSSetShaderResources(0, _countof(pssrvs), pssrvs);

    D3D11_MAPPED_SUBRESOURCE map;
    if (SUCCEEDED(pD3DContext->Map(_vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
    {
        VSConstants* pVSData = static_cast<VSConstants*>(map.pData);
        pVSData->_width = static_cast<float>(_width);
        pVSData->_height = static_cast<float>(_height);
        pVSData->_minZmm = _minZmm;
        pVSData->_maxZmm = _maxZmm;
        if (rampMode == RampMode::None && nullptr != pTexture)
        {
            UINT width = static_cast<UINT>(pTexture->Width);
            UINT height = static_cast<UINT>(pTexture->Height);
            pVSData->_textureWidth = static_cast<float>(width);
            pVSData->_textureHeight = static_cast<float>(height);
        }
        else
        {
            pVSData->_textureWidth = static_cast<float>(_width);
            pVSData->_textureHeight = static_cast<float>(_height);
        }
        pVSData->_vertexMode = static_cast<float>(vertexMode);
        pD3DContext->Unmap(_vsConstantBuffer.Get(), 0);
    }

    if (SUCCEEDED(pD3DContext->Map(_psConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
    {
        PSConstants* pPSData = static_cast<PSConstants*>(map.pData);
        pPSData->_color = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
        pPSData->_invW = 1.0f / _width;
        pPSData->_invH = 1.0f / _height;
        pPSData->_minZmm = _minZmm;
        pPSData->_maxZmm = _maxZmm;
        pPSData->_vertexMode = static_cast<float>(vertexMode);
        pPSData->_rampMode = static_cast<float>(rampMode);
        if (rampMode == RampMode::None && pTexture != nullptr)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC desc;
            pTexture->TextureSRV->GetDesc(&desc);
            // check for yuv format
            if (desc.Format == DXGI_FORMAT_G8R8_G8B8_UNORM)
            {
                pPSData->_useSurfaceTexture = SURFACE_TEXTURE_MODE_YUV;
            }
            else
            {
                pPSData->_useSurfaceTexture = SURFACE_TEXTURE_MODE_DIRECT;
            }
        }
        else
        {
            pPSData->_useSurfaceTexture = SURFACE_TEXTURE_MODE_NONE;
        }

        const PrimitiveParameter DefaultEffectParameter = { 0 };
        if (nullptr == pEffect)
        {
            pEffect = &DefaultEffectParameter;
        }

        pPSData->_enableLight = pEffect->_enableLighting ? 1.0f : 0.0f;

        BOOL forceTexture =
            nullptr != pTexture &&
            (vertexMode == VertexMode::Surface ||
            vertexMode == VertexMode::SurfaceWithUV ||
            vertexMode == VertexMode::PointSprite);

        pPSData->_enableTexture = (pEffect->_enableTexture || forceTexture) ? 1.0f : 0.0f;
        pPSData->_specularPower = pEffect->_power;

        BOOL ambientOnlyLighting =
            XMVector4Equal(pEffect->_diffuse, XMVectorZero()) &&
            XMVector4Equal(pEffect->_specular, XMVectorZero());

        pPSData->_ambientOnlyLighting = ambientOnlyLighting ? 1.0f : 0.0f;
        pPSData->_ambient = pEffect->_ambient;
        pPSData->_diffuse = pEffect->_diffuse;
        pPSData->_specular = pEffect->_specular;
        pPSData->_lightDir = pEffect->_direction;
        pPSData->_enableMultiplyColor = pEffect->_enableMultiplyColor ? 1.0f : -1.0f;
        pPSData->_multiplyColor = pEffect->_multiplyColor;
        pD3DContext->Unmap(_psConstantBuffer.Get(), 0);
    }

    // set constant buffers to VS cb1
    pD3DContext->VSSetConstantBuffers(1, 1, _vsConstantBuffer.GetAddressOf());

    pD3DContext->VSSetSamplers(0, 1, _vsSampler.GetAddressOf());

    // set constant buffers to PS cb0
    pD3DContext->PSSetConstantBuffers(0, 1, _psConstantBuffer.GetAddressOf());

    // set constant buffers to GS cb0
    bool pointSprite = (vertexMode == VertexMode::PointSprite || vertexMode == VertexMode::PointSpriteWithCameraRotation);
    pD3DContext->GSSetShader(pointSprite ? _geometryShader.Get() : nullptr, nullptr, 0);

    if (_gsConstantBuffer != nullptr)
    {
        if (SUCCEEDED(pD3DContext->Map(_gsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
        {
            GSConstants* pGSData = static_cast<GSConstants*>(map.pData);
            assert(sizeof(pGSData->_proj) == sizeof(XMMATRIX));
            if (vertexMode == VertexMode::PointSpriteWithCameraRotation)
            {
                memcpy(pGSData->_proj, (view * proj).r, sizeof(pGSData->_proj));
            }
            else
            {
                memcpy(pGSData->_proj, proj.r, sizeof(pGSData->_proj));
            }
            pGSData->_pixelSize = _pixelSizemm;
            pD3DContext->Unmap(_gsConstantBuffer.Get(), 0);
        }
        pD3DContext->GSSetConstantBuffers(0, 1, _gsConstantBuffer.GetAddressOf());
    }

    return TRUE;
}

void DepthMeshEffect::ApplyTransformAll(
    _In_ ID3D11DeviceContext1* const pD3DContext,
    _In_ const XMMATRIX& worldViewMatrix,
    _In_ const XMMATRIX& worldViewProjectionMatrix)
{
    if (nullptr == _cameraConstantBuffer)
    {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE map;
    if (SUCCEEDED(pD3DContext->Map(_cameraConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
    {
        MatrixViewConstants* pData = static_cast<MatrixViewConstants*>(map.pData);
        pData->_wvp = worldViewProjectionMatrix;
        pData->_wv = worldViewMatrix;
        pD3DContext->Unmap(_cameraConstantBuffer.Get(), 0);
    }

    pD3DContext->VSSetConstantBuffers(0, 1, _cameraConstantBuffer.GetAddressOf());
}
