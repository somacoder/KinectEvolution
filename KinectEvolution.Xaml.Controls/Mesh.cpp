//------------------------------------------------------------------------------
// <copyright file="Effect.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "Mesh.h"

using namespace KinectEvolution::Xaml::Controls::Base;

void Mesh::Initialize(_In_ ID3D11Device1* pD3DDevice, UINT numVertices, UINT vertexStride, UINT numIndices, BOOL use16BitIndex)
{
    _numVertices = numVertices;
    _vertexStride = vertexStride;
    _numIndices = numIndices;
    _use16BitIndex = use16BitIndex;

    D3D11_BUFFER_DESC bufferDesc = { 0 };
    bufferDesc.ByteWidth = numVertices * vertexStride;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    DX::ThrowIfFailed(
        pD3DDevice->CreateBuffer(&bufferDesc, nullptr, &_vertexBuffer)
        );

    if (0 < numIndices)
    {
        D3D11_BUFFER_DESC IBDesc = { 0 };
        IBDesc.ByteWidth = numIndices * (use16BitIndex ? sizeof(UINT16) : sizeof(UINT32));
        IBDesc.Usage = D3D11_USAGE_DYNAMIC;
        IBDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        IBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        DX::ThrowIfFailed(
            pD3DDevice->CreateBuffer(&IBDesc, nullptr, &_indexBuffer)
            );
    }
}

void* Mesh::LockVertexBuffer(_In_ ID3D11DeviceContext1* pD3DContext)
{
    D3D11_MAPPED_SUBRESOURCE map;
    HRESULT hr = pD3DContext->Map(_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    return FAILED(hr) ? nullptr : map.pData;
}

void Mesh::UnlockVertexBuffer(_In_ ID3D11DeviceContext1* pD3DContext)
{
    pD3DContext->Unmap(_vertexBuffer.Get(), 0);
}

void* Mesh::LockIndexBuffer(_In_ ID3D11DeviceContext1* pD3DContext)
{
    if (0 == _numIndices)
    {
        return nullptr;
    }

    D3D11_MAPPED_SUBRESOURCE map;
    HRESULT hr = pD3DContext->Map(_indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    return FAILED(hr) ? nullptr : map.pData;
}

void Mesh::UnlockIndexBuffer(_In_ ID3D11DeviceContext1* pD3DContext)
{
    pD3DContext->Unmap(_indexBuffer.Get(), 0);
}

void Mesh::RenderTriangleList(_In_ ID3D11DeviceContext1* pD3DContext, BOOL useIndex)
{
    Render(pD3DContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, useIndex);
}

void Mesh::RenderLineList(_In_ ID3D11DeviceContext1* pD3DContext, BOOL useIndex)
{
    Render(pD3DContext, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, useIndex);
}

void Mesh::RenderPointList(_In_ ID3D11DeviceContext1* pD3DContext)
{
    Render(pD3DContext, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, FALSE);
}

void Mesh::Render(_In_ ID3D11DeviceContext1* pD3DContext, D3D11_PRIMITIVE_TOPOLOGY topology, BOOL useIndex)
{
    pD3DContext->IASetPrimitiveTopology(topology);

    ID3D11Buffer* buffers[] = { _vertexBuffer.Get() };
    UINT strides[] = { _vertexStride };
    UINT offsets[] = { 0 };

    pD3DContext->IASetVertexBuffers(0, 1, buffers, strides, offsets);

    if (_numIndices > 0 && useIndex)
    {
        pD3DContext->IASetIndexBuffer(_indexBuffer.Get(), _use16BitIndex ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
        pD3DContext->DrawIndexed(_numIndices, 0, 0);
    }
    else
    {
        pD3DContext->Draw(_numVertices, 0);
    }
}
