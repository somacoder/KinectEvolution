//------------------------------------------------------------------------------
// <copyright file="DepthPointEffect.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "DepthPointEffect.h"
#include "Utils.h"
#include "shaders.h"

using namespace KinectEvolution::Xaml::Controls::DepthMap;

struct VSConstants
{
    // (b0) w, h, min_z(in mm), max_z(in mm)
    float _width;
    float _height;
    float _minZmm;
    float _maxZmm;

    // (b1) w, h
    float _textureWidth;
    float _textureHeight;
    float _pad1[2];
};

struct PSConstants
{
    // (b0)
    XMVECTOR _color; // rgba of point color when no ramp texture

    // (b1) 1/w, 1/h, min_z(in mm), max_z(in mm)
    float _invW;
    float _invH;
    float _minZmm;
    float _maxZmm;

    // (b2) vertex mode/ ramp mode/ use surface texture/ enable multiply color
    float _vertexMode;
    float _rampMode;
    float _useSurfaceTexture;
    float _enableMultiplyColor;

    // (b3) multiply color
    XMVECTOR _multiplyColor;
};

DepthPointEffect::DepthPointEffect()
    : Effect()
    , _loadingComplete(FALSE)
    , _greyRampTextureSRV(nullptr)
    , _colorRampTextureSRV(nullptr)
    , _vsConstantBuffer(nullptr)
    , _psConstantBuffer(nullptr)
{
}

void DepthPointEffect::Initialize(_In_ ID3D11Device1* const pD3DDevice, UINT width, UINT height, float minZmm, float maxZmm)
{
    // Asynchronously load vertex shader and create input layout.
    auto loadVSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\DepthPointVS.cso");
    auto createVSTask = loadVSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        CONST D3D11_INPUT_ELEMENT_DESC DepthPointVertexElems[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "POSITION", 1, DXGI_FORMAT_R32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        Effect::InitializeVS(pD3DDevice, &fileData[0], fileData.size(), DepthPointVertexElems, _countof(DepthPointVertexElems));

    });

    // Asynchronously load pixel shader and create constant buffer.
    auto loadPSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\DepthPointPS.cso");
    auto createPSTask = loadPSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        Effect::InitializePS(pD3DDevice, &fileData[0], fileData.size());

    });

    auto createShadersTask = (createPSTask && createVSTask).then([this, pD3DDevice, width, height, minZmm, maxZmm]() {

        const UINT RampLevels = 256;

        // create grey texture
        static UINT greyData[RampLevels];
        for (int i = 0; i < RampLevels; i++)
        {
            UINT level = (RampLevels - i - 1) * 256 / RampLevels;
            greyData[i] = 0xFF000000 | (level << 16) | (level << 8) | level;
        }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> pGreyTexture;
        D3D11_TEXTURE2D_DESC tex2DDesc = { 0 };
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));

        tex2DDesc.Width = RampLevels;
        tex2DDesc.Height = 1;
        tex2DDesc.MipLevels = 1;
        tex2DDesc.ArraySize = 1;
        tex2DDesc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
        tex2DDesc.SampleDesc.Count = 1;
        tex2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initData = { 0 };
        initData.pSysMem = greyData;
        initData.SysMemPitch = RampLevels * sizeof(DWORD);

        DX::ThrowIfFailed(
            pD3DDevice->CreateTexture2D(&tex2DDesc, &initData, &pGreyTexture)
            );

        srvDesc.Format = tex2DDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        DX::ThrowIfFailed(
            pD3DDevice->CreateShaderResourceView(pGreyTexture.Get(), &srvDesc, &_greyRampTextureSRV)
            );

        // create color ramp texture
        static DWORD colorData[RampLevels];
        for (int i = 0; i < RampLevels; i++)
        {
            colorData[i] = ColorRamp<D3DCOLOR_ARGB>(static_cast<float>(i) / (RampLevels - 1));
        }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> pColorTexture;
        tex2DDesc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
        initData.pSysMem = colorData;
        initData.SysMemPitch = RampLevels * sizeof(DWORD);

        DX::ThrowIfFailed(
            pD3DDevice->CreateTexture2D(&tex2DDesc, &initData, &pColorTexture)
            );

        srvDesc.Format = tex2DDesc.Format;
        DX::ThrowIfFailed(
            pD3DDevice->CreateShaderResourceView(pColorTexture.Get(), &srvDesc, &_colorRampTextureSRV)
            );

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

        _width = width;
        _height = height;
        _minZmm = minZmm;
        _maxZmm = maxZmm;

    });

    // Once the everything is done, start rendering
    (createPSTask && createVSTask).then([this]() {
        _loadingComplete = true;
    });
}

BOOL DepthPointEffect::Apply(
    _In_ ID3D11DeviceContext1* const pD3DContext,
    VertexMode vertexMode, RampMode rampMode,
    _In_opt_ Texture^ customRampTexture,
    _In_opt_ Texture^ depthTexture)
{
    if (!Effect::Apply(pD3DContext))
    {
        return FALSE;
    }

    ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr };
    switch (rampMode)
    {
    case RampMode::Color:
        srvs[0] = _colorRampTextureSRV.Get();
        break;
    case RampMode::Grey:
        srvs[0] = _greyRampTextureSRV.Get();
        break;
    default:
        if (nullptr != customRampTexture)
        {
            srvs[0] = customRampTexture->TextureSRV;
        }
        break;
    }

    srvs[1] = depthTexture->TextureSRV;

    pD3DContext->PSSetShaderResources(0, _countof(srvs), srvs);

    D3D11_MAPPED_SUBRESOURCE map;
    if (SUCCEEDED(pD3DContext->Map(_vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
    {
        VSConstants* pVSData = static_cast<VSConstants*>(map.pData);
        pVSData->_width = static_cast<float>(_width);
        pVSData->_height = static_cast<float>(_height);
        pVSData->_minZmm = _minZmm;
        pVSData->_maxZmm = _maxZmm;
        if (rampMode == RampMode::None && nullptr != customRampTexture)
        {
            UINT width = static_cast<UINT>(customRampTexture->Width);
            UINT height = static_cast<UINT>(customRampTexture->Height);
            pVSData->_textureWidth = static_cast<float>(width);
            pVSData->_textureHeight = static_cast<float>(height);
        }
        else
        {
            pVSData->_textureWidth = static_cast<float>(_width);
            pVSData->_textureHeight = static_cast<float>(_height);
        }

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
        if (rampMode == RampMode::None && nullptr != customRampTexture)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC desc;
            customRampTexture->TextureSRV->GetDesc(&desc);

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

        pPSData->_enableMultiplyColor = -1.0f;
        //if (pMultiplyColor != nullptr)
        //{
        //    pPSData->_multiplyColor = *pMultiplyColor;
        //    pPSData->_enableMultiplyColor = 1.0f;
        //}

        pD3DContext->Unmap(_psConstantBuffer.Get(), 0);
    }

    // set constant buffers to VS cb1
    pD3DContext->VSSetConstantBuffers(1, 1, _vsConstantBuffer.GetAddressOf());

    // set constant buffers to PS cb0
    pD3DContext->PSSetConstantBuffers(0, 1, _psConstantBuffer.GetAddressOf());

    return TRUE;
}
