//------------------------------------------------------------------------------
// <copyright file="RampEffect.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "RampEffect.h"

using namespace KinectEvolution::Xaml::Controls::Base;

RampEffect::RampEffect()
    : Effect()
    , _constantBuffer(nullptr)
    , _geometryShader(nullptr)
{
}

void RampEffect::Initialize(_In_ ID3D11Device1* const pD3DDevice)
{

    // Asynchronously load vertex shader and create input layout.
    auto loadVSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\RampEffectVS.cso");
    auto createVSTask = loadVSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        Effect::InitializeVS(pD3DDevice, &fileData[0], fileData.size(), vertexDesc, _countof(vertexDesc));

    });

    // Asynchronously load pixel shader and create constant buffer.
    auto loadGSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\RampEffectGS.cso");
    auto createGSTask = loadGSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        DX::ThrowIfFailed(
            pD3DDevice->CreateGeometryShader(
            &fileData[0],
            fileData.size(),
            nullptr,
            &_geometryShader));

    });

    // Asynchronously load pixel shader and create constant buffer.
    auto loadPSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\RampEffectPS.cso");
    auto createPSTask = loadPSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        Effect::InitializePS(pD3DDevice, &fileData[0], fileData.size());

    });

    auto createShadersTask = (createVSTask && createGSTask&& createPSTask).then([this, pD3DDevice]() {

        // create constant buffers
        D3D11_BUFFER_DESC CBDesc = { 0 };
        CBDesc.ByteWidth = sizeof(RampParameter);
        CBDesc.Usage = D3D11_USAGE_DYNAMIC;
        CBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        CBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        DX::ThrowIfFailed(
            pD3DDevice->CreateBuffer(&CBDesc, NULL, &_constantBuffer)
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

BOOL RampEffect::Apply(
    _In_ ID3D11DeviceContext1* const pD3DContext,
    _In_ ID3D11ShaderResourceView* pIRSRV,
    _In_ ID3D11ShaderResourceView* pIRRampSRV,
    _In_opt_ const RampParameter* pParam)
{
    if (!Effect::Apply(pD3DContext))
    {
        return FALSE;
    };

    // set GS
    pD3DContext->GSSetShader(_geometryShader.Get(), nullptr, 0);

    static const RampParameter DefaultParam = { XMVectorZero(), 1.0f, 0.0f, FLT_MAX };
    if (nullptr == pParam)
    {
        pParam = &DefaultParam;
    }

    D3D11_MAPPED_SUBRESOURCE map;
    if (SUCCEEDED(pD3DContext->Map(_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
    {
        RampParameter* pData = static_cast<RampParameter*>(map.pData);
        pData->_outRangeValue = pParam->_outRangeValue;
        pData->_multiplier = pParam->_multiplier;
        pData->_rangeBegin = pParam->_rangeBegin;
        pData->_rangeWrap = pParam->_rangeWrap;
        pData->_rampLevels = pParam->_rampLevels;
        pD3DContext->Unmap(_constantBuffer.Get(), 0);
    }

    // set constant buffers to PS cb0
    pD3DContext->PSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());

    ID3D11ShaderResourceView* srvs[] = { pIRSRV, pIRRampSRV };
    pD3DContext->PSSetShaderResources(0, _countof(srvs), srvs);

    // ramp lookup texture or surface texture s0
    pD3DContext->PSSetSamplers(0, 1, _textureSampler.GetAddressOf());

    return TRUE;
}

