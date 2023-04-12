//------------------------------------------------------------------------------
// <copyright file="BlockManMesh.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "BlockManMesh.h"

using namespace KinectEvolution::Xaml::Controls::BlockMan;

BlockManMesh::BlockManMesh()
    : Mesh()
    , _loadingComplete(FALSE)
    , _blockManFX(nullptr)
{
}

void BlockManMesh::Initialize(_In_ ID3D11Device1* pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext)
{
    // set the effect for mesh
    _blockManFX = ref new BlockManEffect();
    _blockManFX->Initialize(pD3DDevice);

    // create a cube centered at (0,0,0) with size 1.0f
    Mesh::Initialize(pD3DDevice, 24, sizeof(BlockManVertex), 36, TRUE);

    BlockManVertex* pVertex = reinterpret_cast<BlockManVertex*>(Mesh::LockVertexBuffer(pD3DContext));
    if (nullptr == pVertex)
    {
        return;
    }

    UINT16* pIndex = reinterpret_cast<UINT16*>(Mesh::LockIndexBuffer(pD3DContext));
    if (nullptr == pIndex)
    {
        Mesh::UnlockVertexBuffer(pD3DContext);
        return;
    }

    // +Y (top face)
    *pVertex++ = { { -0.5f, 1.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.25f, 0.00f } };
    *pVertex++ = { { 0.5f, 1.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.50f, 0.00f } };
    *pVertex++ = { { 0.5f, 1.0f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.50f, 0.25f } };
    *pVertex++ = { { -0.5f, 1.0f, 0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.25f, 0.25f } };
    *pIndex++ = 0;
    *pIndex++ = 1;
    *pIndex++ = 2;
    *pIndex++ = 0;
    *pIndex++ = 2;
    *pIndex++ = 3;

    // -Y (bottom face)
    *pVertex++ = { { -0.5f, 0.0f, 0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.25f, 0.50f } };
    *pVertex++ = { { 0.5f, 0.0f, 0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.50f, 0.50f } };
    *pVertex++ = { { 0.5f, 0.0f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.50f, 0.75f } };
    *pVertex++ = { { -0.5f, 0.0f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 0.25f, 0.75f } };
    *pIndex++ = 4;
    *pIndex++ = 5;
    *pIndex++ = 6;
    *pIndex++ = 4;
    *pIndex++ = 6;
    *pIndex++ = 7;

    // +X (Right face)
    *pVertex++ = { { 0.5f, 1.0f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.50f, 0.25f } };
    *pVertex++ = { { 0.5f, 1.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.75f, 0.25f } };
    *pVertex++ = { { 0.5f, 0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.75f, 0.50f } };
    *pVertex++ = { { .5f, 0.0f, 0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.50f, 0.50f } };
    *pIndex++ = 8;
    *pIndex++ = 9;
    *pIndex++ = 10;
    *pIndex++ = 8;
    *pIndex++ = 10;
    *pIndex++ = 11;

    // -X (Left face)
    *pVertex++ = { { -0.5f, 1.0f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.00f, 0.25f } };
    *pVertex++ = { { -0.5f, 1.0f, 0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.25f, 0.25f } };
    *pVertex++ = { { -0.5f, 0.0f, 0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.25f, 0.50f } };
    *pVertex++ = { { -0.5f, 0.0f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 0.00f, 0.50f } };
    *pIndex++ = 12;
    *pIndex++ = 13;
    *pIndex++ = 14;
    *pIndex++ = 12;
    *pIndex++ = 14;
    *pIndex++ = 15;

    // +Z (front face)
    *pVertex++ = { { -0.5f, 1.0f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.25f, 0.75f } };
    *pVertex++ = { { 0.5f, 1.0f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.50f, 0.75f } };
    *pVertex++ = { { 0.5f, 0.0f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.50f, 1.0f } };
    *pVertex++ = { { -0.5f, 0.0f, 0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.25f, 1.0f } };
    *pIndex++ = 16;
    *pIndex++ = 17;
    *pIndex++ = 18;
    *pIndex++ = 16;
    *pIndex++ = 18;
    *pIndex++ = 19;

    // -Z (front face)
    *pVertex++ = { { 0.5f, 1.0f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.25f, 0.25f } };
    *pVertex++ = { { -0.5f, 1.0f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.50f, 0.25f } };
    *pVertex++ = { { -0.5f, 0.0f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.50f, 0.50f } };
    *pVertex++ = { { 0.5f, 0.0f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 0.25f, 0.50f } };
    *pIndex++ = 20;
    *pIndex++ = 21;
    *pIndex++ = 22;
    *pIndex++ = 20;
    *pIndex++ = 22;
    *pIndex++ = 23;

    Mesh::UnlockVertexBuffer(pD3DContext);
    Mesh::UnlockIndexBuffer(pD3DContext);
}

void BlockManMesh::Render(_In_ ID3D11DeviceContext1* pD3DContext, BOOL useIndex, _In_ BlockManEffect::BlockManParams* pParams)
{
    if (!_blockManFX->Apply(pD3DContext, pParams))
    {
        return;
    }

    Mesh::RenderTriangleList(pD3DContext, useIndex);
}
