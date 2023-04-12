//------------------------------------------------------------------------------
// <copyright file="Effect.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "Effect.h"

using namespace KinectEvolution::Xaml::Controls::Base;

Effect::Effect()
{
}

void Effect::InitializeVS(
    _In_ ID3D11Device1* const pD3DDevice,
    _In_reads_bytes_(vertexShaderByteCodeLength) const void* pVertexShaderByteCode,
    _In_ SIZE_T vertexShaderByteCodeLength,
    _In_reads_(numInputElementDescs) const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
    _In_ UINT numInputElementDescs)
{
    DX::ThrowIfFailed(
        pD3DDevice->CreateVertexShader(pVertexShaderByteCode, vertexShaderByteCodeLength, nullptr, &_vertexShader)
        );

    DX::ThrowIfFailed(
        pD3DDevice->CreateInputLayout(pInputElementDescs, numInputElementDescs, pVertexShaderByteCode, vertexShaderByteCodeLength, &_inputLayout)
        );
}

void Effect::InitializePS(
    _In_ ID3D11Device1* const pD3DDevice,
    _In_reads_bytes_(pixelShaderByteCodeLength) const void* pPixelShaderByteCode,
    _In_ SIZE_T pixelShaderByteCodeLength)
{
    DX::ThrowIfFailed(
        pD3DDevice->CreatePixelShader(pPixelShaderByteCode, pixelShaderByteCodeLength, nullptr, &_pixelShader)
        );
}

BOOL Effect::Apply(_In_ ID3D11DeviceContext1* const pD3DContext)
{
    if (!IsLoadingComplete())
    {
        return FALSE;
    }

    // unbind previous shaders
    pD3DContext->IASetInputLayout(nullptr);
    pD3DContext->VSSetShader(nullptr, nullptr, 0);
    pD3DContext->GSSetShader(nullptr, nullptr, 0);
    pD3DContext->PSSetShader(nullptr, nullptr, 0);

    // Set input assembler state
    pD3DContext->IASetInputLayout(_inputLayout.Get());

    // Apply vertex and pixel shader
    pD3DContext->VSSetShader(_vertexShader.Get(), nullptr, 0);
    pD3DContext->PSSetShader(_pixelShader.Get(), nullptr, 0);

    return TRUE;
}
