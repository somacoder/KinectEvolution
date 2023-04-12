//------------------------------------------------------------------------------
// <copyright file="PrimitiveMesh.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "PrimitiveMesh.h"

using namespace KinectEvolution::Xaml::Controls::Base;

Platform::Array<PrimitiveMesh^>^ PrimitiveMesh::Default
= ref new Platform::Array<PrimitiveMesh^>(static_cast<UINT>(DefaultPrimitive::Count));

PrimitiveMesh^ PrimitiveMesh::GetDefaultPrimitive(
    _In_ ID3D11Device1* const pD3DDevice,
    _In_ ID3D11DeviceContext1* const pD3DContext,
    DefaultPrimitive primitiveType)
{
    PrimitiveMesh^ mesh = PrimitiveMesh::Default[static_cast<UINT>(primitiveType)];

    if (nullptr == mesh || !mesh->IsLoadingComplete())
    {
        switch (primitiveType)
        {
        case DefaultPrimitive::Cube:
        {
            mesh = ref new CubeMesh();
            CubeMesh^ cube = static_cast<CubeMesh^>(mesh);
            cube->Initialize(pD3DDevice, pD3DContext);
            break;
        }
        case DefaultPrimitive::Sphere:
        {
            mesh = ref new SphereMesh();
            SphereMesh^ sphere = static_cast<SphereMesh^>(mesh);
            sphere->Initialize(pD3DDevice, pD3DContext, 20);
            break;
        }
        case DefaultPrimitive::Cylinder:
        {
            mesh = ref new CylinderMesh();
            CylinderMesh^ cylinder = static_cast<CylinderMesh^>(mesh);
            cylinder->Initialize(pD3DDevice, pD3DContext, 20);
            break;
        }
        case DefaultPrimitive::Cone:
        {
            mesh = ref new ConeMesh();
            ConeMesh^ cone = static_cast<ConeMesh^>(mesh);
            cone->Initialize(pD3DDevice, pD3DContext, 0.3f, 20);
            break;
        }
        case DefaultPrimitive::Pyramid:
        {
            mesh = ref new PyramidMesh();
            PyramidMesh^ pyramid = static_cast<PyramidMesh^>(mesh);
            pyramid->Initialize(pD3DDevice, pD3DContext, 1.0f);
            break;
        }
        default:
            return nullptr;
        }
    }

    return mesh;
}

void CubeMesh::Initialize(_In_ ID3D11Device1* const pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext)
{
    // create a cube centered at (0,0,0) with size 1.0f
    Mesh::Initialize(pD3DDevice, 24, sizeof(PrimitiveVertex), 36, TRUE);

    PrimitiveVertex* pVertex = reinterpret_cast<PrimitiveVertex*>(Mesh::LockVertexBuffer(pD3DContext));
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

    UINT16 face = 0;

    // down
    *pVertex++ = PrimitiveVertex( 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex( 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0xFFFFFFFF);
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 1;
    *pIndex++ = face * 4 + 2;
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 2;
    *pIndex++ = face * 4 + 3;
    face++;

    // up
    *pVertex++ = PrimitiveVertex( 0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex( 0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0xFFFFFFFF);
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 2;
    *pIndex++ = face * 4 + 1;
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 3;
    *pIndex++ = face * 4 + 2;
    face++;

    // left
    *pVertex++ = PrimitiveVertex(-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0xFFFFFFFF);
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 1;
    *pIndex++ = face * 4 + 2;
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 2;
    *pIndex++ = face * 4 + 3;
    face++;

    // right
    *pVertex++ = PrimitiveVertex(0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0xFFFFFFFF);
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 2;
    *pIndex++ = face * 4 + 1;
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 3;
    *pIndex++ = face * 4 + 2;
    face++;

    // back
    *pVertex++ = PrimitiveVertex( 0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex( 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0xFFFFFFFF);
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 1;
    *pIndex++ = face * 4 + 2;
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 2;
    *pIndex++ = face * 4 + 3;
    face++;

    // front
    *pVertex++ = PrimitiveVertex( 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex( 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0xFFFFFFFF);
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 2;
    *pIndex++ = face * 4 + 1;
    *pIndex++ = face * 4;
    *pIndex++ = face * 4 + 3;
    *pIndex++ = face * 4 + 2;
    face++;

    Mesh::UnlockVertexBuffer(pD3DContext);
    Mesh::UnlockIndexBuffer(pD3DContext);
}

// create a sphere centered at (0,0,0) with diameter of 1.0f
void SphereMesh::Initialize(_In_ ID3D11Device1* const pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext, UINT segments)
{
    ASSERT(segments <= 100);

    Mesh::Initialize(pD3DDevice, segments * segments, sizeof(PrimitiveVertex), segments * (segments - 1) * 6, TRUE);

    PrimitiveVertex* pVertex = reinterpret_cast<PrimitiveVertex*>(Mesh::LockVertexBuffer(pD3DContext));
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

    for (UINT y = 0; y < segments; y++)
    {
        float latitude = XM_PI * y / (segments - 1) - XM_PI / 2;

        for (UINT x = 0; x < segments; x++)
        {
            float longitude = 2 * XM_PI * x / segments;
            float nx = cosf(latitude) * sinf(longitude);
            float ny = sinf(latitude);
            float nz = cosf(latitude) * cosf(longitude);

            *pVertex++ = PrimitiveVertex(0.5f * nx, 0.5f * ny, 0.5f * nz, nx, ny, nz, 0xFFFFFFFF);

            if (y == 0)
            {
                continue;
            }

            UINT16 i0;
            UINT16 i1;
            UINT16 i2;
            UINT16 i3;

            if (x > 0)
            {
                i0 = static_cast<UINT16>((y - 1) * segments + x - 1);
                i1 = static_cast<UINT16>((y - 1) * segments + x);
                i2 = static_cast<UINT16>(y * segments + x);
                i3 = static_cast<UINT16>(y * segments + x - 1);
            }
            else
            {
                i0 = static_cast<UINT16>((y - 1) * segments + segments - 1);
                i1 = static_cast<UINT16>((y - 1) * segments);
                i2 = static_cast<UINT16>(y * segments);
                i3 = static_cast<UINT16>(y * segments + segments - 1);
            }

            *pIndex++ = i0;
            *pIndex++ = i1;
            *pIndex++ = i2;

            *pIndex++ = i0;
            *pIndex++ = i2;
            *pIndex++ = i3;
        }
    }

    Mesh::UnlockVertexBuffer(pD3DContext);
    Mesh::UnlockIndexBuffer(pD3DContext);
}

void CylinderMesh::Initialize(_In_ ID3D11Device1* const pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext, UINT segments)
{
    ASSERT(segments <= 100);

    Mesh::Initialize(pD3DDevice, 2 * segments + 2, sizeof(PrimitiveVertex), 12 * segments, TRUE);

    PrimitiveVertex* pVertex = reinterpret_cast<PrimitiveVertex*>(Mesh::LockVertexBuffer(pD3DContext));
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

    for (UINT x = 0; x < segments; x++)
    {
        float angle = 2 * XM_PI * x / segments;
        float nx = sinf(angle);
        float ny = 0;
        float nz = cosf(angle);

        // tube

        *pVertex++ = PrimitiveVertex(nx, 0.0f, nz, nx, ny, nz, 0xFFFFFFFF);
        *pVertex++ = PrimitiveVertex(nx, 1.0f, nz, nx, ny, nz, 0xFFFFFFFF);

        UINT16 i0;
        UINT16 i1;
        UINT16 i2;
        UINT16 i3;

        if (x > 0)
        {
            i0 = static_cast<UINT16>(2 * x - 2);
            i1 = static_cast<UINT16>(2 * x);
            i2 = static_cast<UINT16>(2 * x + 1);
            i3 = static_cast<UINT16>(2 * x - 1);
        }
        else
        {
            i0 = static_cast<UINT16>(2 * segments - 2);
            i1 = 0;
            i2 = 1;
            i3 = static_cast<UINT16>(2 * segments - 1);
        }

        *pIndex++ = i0;
        *pIndex++ = i1;
        *pIndex++ = i2;

        *pIndex++ = i0;
        *pIndex++ = i2;
        *pIndex++ = i3;

        // cap

        *pIndex++ = static_cast<UINT16>(segments * 2);
        *pIndex++ = i1;
        *pIndex++ = i0;

        *pIndex++ = static_cast<UINT16>(segments * 2 + 1);
        *pIndex++ = i3;
        *pIndex++ = i2;
    }

    *pVertex++ = PrimitiveVertex(0, 0.0f, 0, 0, -1.0f, 0, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(0, 1.0f, 0, 0, 1.0f, 0, 0xFFFFFFFF);

    Mesh::UnlockVertexBuffer(pD3DContext);
    Mesh::UnlockIndexBuffer(pD3DContext);
}

// need this to calculate normal correctly, note when applying non-uniform (different in x/y/z) scale the visual effect might be incorrect for normal
void ConeMesh::Initialize(_In_ ID3D11Device1* const pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext, float baseRadius, UINT segments)
{
    // create a cone center from (0,0,0) to (0,1,0) with baseRadius, with (0,1,0) being the pointing end
    ASSERT(segments <= 100);

    const float waist[] = { 0.5f, 0.75f, 0.875f };
    const int NumVertexPerSegment = 3 + _countof(waist);
    Mesh::Initialize(pD3DDevice, NumVertexPerSegment * segments + 1, sizeof(PrimitiveVertex), 6 * (1 + _countof(waist)) * segments, TRUE);

    PrimitiveVertex* pVertex = reinterpret_cast<PrimitiveVertex*>(Mesh::LockVertexBuffer(pD3DContext));
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

    float invSqrt1nR2 = 1 / sqrtf(1 + baseRadius * baseRadius);
    float rInvSqrt1nR2 = baseRadius / sqrtf(1 + baseRadius * baseRadius);

    for (UINT x = 0; x < segments; x++)
    {
        // the vertex on cone base
        float angle = 2 * XM_PI * x / segments;
        float nx = sinf(angle) * invSqrt1nR2;
        float ny = rInvSqrt1nR2;
        float nz = cosf(angle) * invSqrt1nR2;
        *pVertex++ = PrimitiveVertex(nx * baseRadius, 0.0f, nz * baseRadius, 0, -1, 0, 0xFFFFFFFF);
        *pVertex++ = PrimitiveVertex(nx * baseRadius, 0.0f, nz * baseRadius, nx, ny, nz, 0xFFFFFFFF);

        // the vertex on cone body
        for (int i = 0; i < _countof(waist); i++)
        {
            float waistRadius = (1 - waist[i]) * baseRadius;
            *pVertex++ = PrimitiveVertex(nx * waistRadius, waist[i], nz * waistRadius, nx, ny, nz, 0xFFFFFFFF);
        }

        // the vertex on cone tip
        float angleTip = 2 * XM_PI * (x + 0.5f) / segments;
        float tipnx = sinf(angleTip) * invSqrt1nR2;
        float tipnz = cosf(angleTip) * invSqrt1nR2;
        *pVertex++ = PrimitiveVertex(0.0f, 1.0f, 0.0f, tipnx, ny, tipnz, 0xFFFFFFFF);

        // generate triangles, start from the base
        UINT16 iBase0 = static_cast<UINT16>(NumVertexPerSegment * x);
        UINT16 iBase1 = (x < segments - 1) ? iBase0 + NumVertexPerSegment : 0;

        // cap
        *pIndex++ = iBase0;
        *pIndex++ = static_cast<UINT16>(NumVertexPerSegment * segments);
        *pIndex++ = iBase1;
        iBase0++;
        iBase1++;

        // body
        for (int i = 0; i < _countof(waist); i++)
        {
            UINT16 iUpper0 = iBase0 + 1;
            UINT16 iUpper1 = iBase1 + 1;

            *pIndex++ = iBase0;
            *pIndex++ = iBase1;
            *pIndex++ = iUpper1;

            *pIndex++ = iBase0;
            *pIndex++ = iUpper1;
            *pIndex++ = iUpper0;

            iBase0++;
            iBase1++;
        }

        // tip
        *pIndex++ = iBase0;
        *pIndex++ = iBase1;
        *pIndex++ = iBase0 + 1;
    }

    *pVertex++ = PrimitiveVertex(0, 0, 0, 0, -1.0f, 0, 0xFFFFFFFF);

    Mesh::UnlockVertexBuffer(pD3DContext);
    Mesh::UnlockIndexBuffer(pD3DContext);
}

static void AddTriangle(
    _In_ const XMVECTOR& p0,
    _In_ const XMVECTOR& p1,
    _In_ const XMVECTOR& p2,
    _Inout_ UINT16& baseIndex,
    _Inout_ PrimitiveVertex* &pVertex,
    _Inout_ UINT16* &pIndex)
{
    float x0 = XMVectorGetX(p0);
    float y0 = XMVectorGetY(p0);
    float z0 = XMVectorGetZ(p0);

    float x1 = XMVectorGetX(p1);
    float y1 = XMVectorGetY(p1);
    float z1 = XMVectorGetZ(p1);

    float x2 = XMVectorGetX(p2);
    float y2 = XMVectorGetY(p2);
    float z2 = XMVectorGetZ(p2);

    XMVECTOR normal = XMVector3Cross(p1 - p0, p2 - p0);
    float nx = XMVectorGetX(normal);
    float ny = XMVectorGetY(normal);
    float nz = XMVectorGetZ(normal);

    *pVertex++ = PrimitiveVertex(x0, y0, z0, nx, ny, nz, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(x1, y1, z1, nx, ny, nz, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(x2, y2, z2, nx, ny, nz, 0xFFFFFFFF);

    *pIndex++ = baseIndex;
    *pIndex++ = baseIndex + 1;
    *pIndex++ = baseIndex + 2;

    baseIndex += 3;
}

void PyramidMesh::Initialize(_In_ ID3D11Device1* const pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext, float baseSize)
{
    Mesh::Initialize(pD3DDevice, 16, sizeof(PrimitiveVertex), 18, TRUE);

    PrimitiveVertex* pVertex = reinterpret_cast<PrimitiveVertex*>(Mesh::LockVertexBuffer(pD3DContext));
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

    float halfBaseSize = baseSize / 2;
    XMVECTOR baseNegXNegZ = XMVectorSet(-halfBaseSize, 0, -halfBaseSize, 0);
    XMVECTOR basePosXNegZ = XMVectorSet(halfBaseSize, 0, -halfBaseSize, 0);
    XMVECTOR basePosXPosZ = XMVectorSet(halfBaseSize, 0, halfBaseSize, 0);
    XMVECTOR baseNegXPosZ = XMVectorSet(-halfBaseSize, 0, halfBaseSize, 0);
    XMVECTOR tip = XMVectorSet(0, 1, 0, 0);

    // siding
    UINT16 baseIndex = 0;
    AddTriangle(tip, basePosXNegZ, baseNegXNegZ, baseIndex, pVertex, pIndex);
    AddTriangle(tip, baseNegXNegZ, baseNegXPosZ, baseIndex, pVertex, pIndex);
    AddTriangle(tip, baseNegXPosZ, basePosXPosZ, baseIndex, pVertex, pIndex);
    AddTriangle(tip, basePosXPosZ, basePosXNegZ, baseIndex, pVertex, pIndex);

    // base

    *pVertex++ = PrimitiveVertex(halfBaseSize, 0, -halfBaseSize, 0, -1, 0, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-halfBaseSize, 0, -halfBaseSize, 0, -1, 0, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(-halfBaseSize, 0, halfBaseSize, 0, -1, 0, 0xFFFFFFFF);
    *pVertex++ = PrimitiveVertex(halfBaseSize, 0, halfBaseSize, 0, -1, 0, 0xFFFFFFFF);

    UINT16 i0 = baseIndex;
    UINT16 i1 = baseIndex + 1;
    UINT16 i2 = baseIndex + 2;
    UINT16 i3 = baseIndex + 3;

    *pIndex++ = i0;
    *pIndex++ = i2;
    *pIndex++ = i1;

    *pIndex++ = i0;
    *pIndex++ = i3;
    *pIndex++ = i2;

    Mesh::UnlockVertexBuffer(pD3DContext);
    Mesh::UnlockIndexBuffer(pD3DContext);
}
