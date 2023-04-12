//------------------------------------------------------------------------------
// <copyright file="PrimitiveEffect.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "PrimitiveEffect.h"

using namespace KinectEvolution::Xaml::Controls::Base;

struct LightConstants
{
    // CONST 0      enabled/textured
    float _enableLight;
    float _notused;
    float _specularPower;
    float _ambientOnlyLighting;

    // CONST 1-3    ambient/diffuse/specular color
    XMVECTOR _ambient;
    XMVECTOR _diffuse;
    XMVECTOR _specular;

    // CONST 4      light direction
    XMVECTOR _lightDir;
};

PrimitiveEffect::PrimitiveEffect()
    : Effect()
    , _lightConstantBuffer(nullptr)
{
}

void PrimitiveEffect::Initialize(_In_ ID3D11Device1* const pD3DDevice)
{
    // Asynchronously load vertex shader and create input layout.
    auto loadVSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\PrimitiveVS.cso");
    auto createVSTask = loadVSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        CONST D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        Effect::InitializeVS(pD3DDevice, &fileData[0], fileData.size(), vertexDesc, _countof(vertexDesc));

    });

    // Asynchronously load pixel shader and create constant buffer.
    auto loadPSTask = DX::ReadDataAsync(L"KinectEvolution.Xaml.Controls\\PrimitivePS.cso");
    auto createPSTask = loadPSTask.then([this, pD3DDevice](const std::vector<byte>& fileData) {

        Effect::InitializePS(pD3DDevice, &fileData[0], fileData.size());

    });

    auto createShadersTask = (createPSTask && createVSTask).then([this, pD3DDevice]() {

        // create constant buffers
        D3D11_BUFFER_DESC CBDesc = { 0 };
        CBDesc.ByteWidth = sizeof(LightConstants);
        CBDesc.Usage = D3D11_USAGE_DYNAMIC;
        CBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        CBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        DX::ThrowIfFailed(
            pD3DDevice->CreateBuffer(&CBDesc, nullptr, &_lightConstantBuffer)
            );

        CBDesc.ByteWidth = sizeof(MatrixViewConstants);
        DX::ThrowIfFailed(
            pD3DDevice->CreateBuffer(&CBDesc, nullptr, &_cameraConstantBuffer)
            );

    });

    // Once the everything is dine, we can start rendering
    createShadersTask.then([this]() {
        _loadingComplete = true;
    });
}

BOOL PrimitiveEffect::Apply(
    _In_ ID3D11DeviceContext1* const pD3DContext,
    _In_opt_ const PrimitiveParameter* pParam)
{
    if (!Effect::Apply(pD3DContext))
    {
        return FALSE;
    }

    const PrimitiveParameter DefaultParam = { 0 };
    if (nullptr == pParam)
    {
        pParam = &DefaultParam;
    }

    D3D11_MAPPED_SUBRESOURCE map;
    if (SUCCEEDED(pD3DContext->Map(_lightConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map)))
    {
        LightConstants* pData = static_cast<LightConstants*>(map.pData);
        pData->_enableLight = pParam->_enableLighting ? 1.0f : 0.0f;
        pData->_specularPower = pParam->_power;

        BOOL ambientOnlyLighting =
            XMVector4Equal(pParam->_diffuse, XMVectorZero()) &&
            XMVector4Equal(pParam->_specular, XMVectorZero());

        pData->_ambientOnlyLighting = ambientOnlyLighting ? 1.0f : 0.0f;
        pData->_ambient = pParam->_ambient;
        pData->_diffuse = pParam->_diffuse;
        pData->_specular = pParam->_specular;
        pData->_lightDir = pParam->_direction;
        pD3DContext->Unmap(_lightConstantBuffer.Get(), 0);
    }

    // set constant buffers to PS cb0
    pD3DContext->PSSetConstantBuffers(0, 1, _lightConstantBuffer.GetAddressOf());

    return TRUE;
}

void PrimitiveEffect::ApplyTransformAll(
    _In_ ID3D11DeviceContext1* const pD3DContext,
    _In_ CXMMATRIX worldViewMatrix,
    _In_ CXMMATRIX worldViewProjectionMatrix)
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

// utility function for calculate unit up vector transform matrix
DirectX::XMMATRIX PrimitiveEffect::TransformUnitUpVector(
    _In_ CXMVECTOR startPosition,
    _In_ CXMVECTOR endPosition,
    BOOL noStretch)
{
    XMVECTOR delta = endPosition - startPosition;
    float deltaLength = XMVectorGetX(XMVector3Length(delta));
    float angle = XM_PI / 2 - asinf(XMVectorGetY(delta) / deltaLength);

    XMMATRIX mat = XMMatrixTranslationFromVector(startPosition);

    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMVECTOR axis = XMVector3Cross(up, delta);
    if (XMVector3Equal(axis, XMVectorZero()))
    {
        axis = XMVectorSet(-1, 0, 0, 0); // use z axis for rotation when end-start lies on the same or opposite direction as up vector
    }
    mat = XMMatrixRotationAxis(axis, angle) * mat;

    return noStretch ? mat : XMMatrixScaling(1.0f, deltaLength, 1.0f) * mat;
}
