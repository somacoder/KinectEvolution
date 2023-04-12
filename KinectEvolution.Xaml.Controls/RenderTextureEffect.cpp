//------------------------------------------------------------------------------
// <copyright file="RenderTextureEffect.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "RenderTextureEffect.h"
#include "shaders.h"

using namespace KinectEvolution::Xaml::Controls::Base;

RenderTextureEffect::RenderTextureEffect()
    : Effect()
    , _constantBuffer(nullptr)
    , _geometryShader(nullptr)
{
}

void RenderTextureEffect::Initialize(_In_ ID3D11Device1* const pD3DDevice)
{

    // Asynchronously load vertex shader and create input layout.
    auto loadVSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\RenderTextureVS.cso");
    auto createVSTask = loadVSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        Effect::InitializeVS(pD3DDevice, &fileData[0], fileData.size(), vertexDesc, _countof(vertexDesc));

    });

    // Asynchronously load pixel shader and create constant buffer.
    auto loadGSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\RenderTextureGS.cso");
    auto createGSTask = loadGSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        DX::ThrowIfFailed(
            pD3DDevice->CreateGeometryShader(
            &fileData[0],
            fileData.size(),
            nullptr,
            &_geometryShader));

    });

    // Asynchronously load pixel shader and create constant buffer.
    auto loadPSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\RenderTexturePS.cso");
    auto createPSTask = loadPSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        Effect::InitializePS(pD3DDevice, &fileData[0], fileData.size());

    });

    auto createShadersTask = (createVSTask && createGSTask&& createPSTask).then([this, pD3DDevice]() {

        // create constant buffers
        D3D11_BUFFER_DESC CBDesc = { 0 };
        CBDesc.ByteWidth = sizeof(RenderTextureEffect::Paramaters);
        CBDesc.Usage = D3D11_USAGE_DYNAMIC;
        CBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        CBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        DX::ThrowIfFailed(
            pD3DDevice->CreateBuffer(&CBDesc, NULL, &_constantBuffer)
            );

        // Set rasterizer state to disable backface culling
        D3D11_RASTERIZER_DESC rasterDesc;
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.CullMode = D3D11_CULL_NONE;
        rasterDesc.FrontCounterClockwise = true;
        rasterDesc.DepthBias = false;
        rasterDesc.DepthBiasClamp = 0;
        rasterDesc.SlopeScaledDepthBias = 0;
        rasterDesc.DepthClipEnable = false;
        rasterDesc.ScissorEnable = false;
        rasterDesc.MultisampleEnable = false;
        rasterDesc.AntialiasedLineEnable = false;

        DX::ThrowIfFailed(
            pD3DDevice->CreateRasterizerState(&rasterDesc, &_rasterState)
            );

        // create the sampler
        D3D11_SAMPLER_DESC sd;
        ZeroMemory(&sd, sizeof(sd));

        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

        sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

        sd.MaxLOD = FLT_MAX;
        sd.MaxAnisotropy = 16;
        sd.ComparisonFunc = D3D11_COMPARISON_NEVER;

        DX::ThrowIfFailed(
            pD3DDevice->CreateSamplerState(&sd, &_textureSampler)
            );

    });

    // Once the everything is dine, we can start rendering
    createShadersTask.then([this]() {
        _loadingComplete = TRUE;
    });
}

BOOL RenderTextureEffect::Apply(
    _In_ ID3D11DeviceContext1* const pD3DContext,
    _In_ ID3D11ShaderResourceView* pSRV,
    _In_ const RenderTextureEffect::Paramaters* pParam)
{
    // Attach shaders, set VS and PS for the effect
    if (!Effect::Apply(pD3DContext))
    {
        return FALSE;
    };

    // apply GS
    pD3DContext->GSSetShader(_geometryShader.Get(), nullptr, 0);

    // update the parameter for shaders
    static const RenderTextureEffect::Paramaters DefaultParam = {
            { SURFACE_TEXTURE_MODE_NONE, 0.0f, 0.0f, 0.0f },
            { 1.0f, 1.0f },
            { 1.0f, 1.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f } }; // black
    if (nullptr == pParam)
    {
        pParam = &DefaultParam;
    }

    D3D11_MAPPED_SUBRESOURCE map;
    if (SUCCEEDED(pD3DContext->Map(_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
    {
        RenderTextureEffect::Paramaters* pData = static_cast<RenderTextureEffect::Paramaters*>(map.pData);
        pData->texParams = pParam->texParams;
        pData->texSize = pParam->texSize;
        pData->ctlSize = pParam->ctlSize;
        pData->color = pParam->color;
        pD3DContext->Unmap(_constantBuffer.Get(), 0);
    }

    // set constant buffers for GS & PS cb0
    pD3DContext->GSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
    pD3DContext->PSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());

    // set PS shader texture resource
    pD3DContext->PSSetShaderResources(0, 1, &pSRV);

    // set the sample for the texture
    pD3DContext->PSSetSamplers(0, 1, _textureSampler.GetAddressOf());

    // turn off culling
    pD3DContext->RSSetState(_rasterState.Get());

    return TRUE;
}
