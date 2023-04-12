//------------------------------------------------------------------------------
// <copyright file="BlockManEffect.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "BlockManEffect.h"
#include "DDSTextureLoader.h"

using namespace KinectEvolution::Xaml::Controls::BlockMan;

BlockManEffect::BlockManEffect()
    : Effect()
    , _loadingComplete(FALSE)
{
}

void BlockManEffect::Initialize(_In_ ID3D11Device1* pD3DDevice)
{
    // Asynchronously load vertex shader and create input layout.
    auto loadVSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\BlockManVS.cso");
    auto createVSTask = loadVSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        Effect::InitializeVS(pD3DDevice, &fileData[0], fileData.size(), vertexDesc, _countof(vertexDesc));
    });

    // Asynchronously load pixel shader and create constant buffer.
    auto loadPSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\BlockManPS.cso");
    auto createPSTask = loadPSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        Effect::InitializePS(pD3DDevice, &fileData[0], fileData.size());

    });

    auto loadTxTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\BlockManTexture.dds");
    auto createTxTask = loadTxTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        CreateDDSTextureFromMemory(pD3DDevice, &fileData[0], fileData.size(), nullptr, &_textureSRV, 0);

    });

    auto createShadersTask = (createPSTask && createVSTask && createTxTask).then([this, pD3DDevice]() {

        D3D11_BUFFER_DESC bufferDesc = { 0 };
        // Create the constant buffer
        bufferDesc.ByteWidth = sizeof(BlockManParams);
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = 0;
        DX::ThrowIfFailed(
            pD3DDevice->CreateBuffer(&bufferDesc, nullptr, &_constantBuffer)
            );

        D3D11_SAMPLER_DESC sdesc;
        sdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sdesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sdesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sdesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sdesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sdesc.MaxAnisotropy = 16;
        sdesc.MinLOD = -D3D11_FLOAT32_MAX;
        sdesc.MaxLOD = D3D11_FLOAT32_MAX;
        sdesc.MipLODBias = 0;

        DX::ThrowIfFailed(
            pD3DDevice->CreateSamplerState(&sdesc, &_textureSampler)
            );
    });

    // Once the everything is dine, all is ready to be rendered.
    createShadersTask.then([this]() {
        _loadingComplete = true;
    });

}

BOOL BlockManEffect::Apply(_In_ ID3D11DeviceContext1* const pD3DDeviceContext, _In_ BlockManParams* pParams)
{
    if (!Effect::Apply(pD3DDeviceContext))
    {
        return FALSE;
    }

    pD3DDeviceContext->PSSetSamplers(0, 1, _textureSampler.GetAddressOf());

    pD3DDeviceContext->PSSetShaderResources(0, 1, _textureSRV.GetAddressOf());

    // update constants
    pD3DDeviceContext->UpdateSubresource(_constantBuffer.Get(), 0, nullptr, pParams, 0, 0);

    pD3DDeviceContext->VSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
    pD3DDeviceContext->PSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());

    return TRUE;
}
