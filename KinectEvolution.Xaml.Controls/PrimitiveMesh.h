//------------------------------------------------------------------------------
// <copyright file="PrimitiveMesh.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once
#include "Mesh.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Base {

                enum class DefaultPrimitive : UINT
                {
                    Cube = 0,
                    Sphere,
                    Cylinder,
                    Cone,
                    Pyramid,
                    Count
                };

                struct PrimitiveVertex
                {
                    float _x, _y, _z;
                    float _nx, _ny, _nz;
                    DWORD _color; // RGBA, where 0xFF000080 means (R = 128, G = 0, B = 0, A = 255), for D3D_FEATURE_LEVEL_9_x compatible

                    PrimitiveVertex(float x, float y, float z, float nx, float ny, float nz, DWORD color)
                        : _x(x), _y(y), _z(z)
                        , _nx(nx), _ny(ny), _nz(nz)
                        , _color(color)
                    {
                    }
                };

                ref class PrimitiveMesh abstract
                    : Mesh
                {
                internal:
                    PrimitiveMesh() : Mesh(), _loadingComplete(FALSE) {}

                    BOOL IsLoadingComplete() override { return _loadingComplete; }

                    virtual DefaultPrimitive GetType() = 0;

                    static PrimitiveMesh^ GetDefaultPrimitive(
                        _In_ ID3D11Device1* const pD3DDevice,
                        _In_ ID3D11DeviceContext1* const pD3DContext,
                        DefaultPrimitive primitiveType);

                    static Platform::Array<PrimitiveMesh^>^ Default;

                private:
                    BOOL _loadingComplete;
                };

                ref class CubeMesh sealed
                    : PrimitiveMesh
                {
                internal:
                    CubeMesh() : PrimitiveMesh(), _type(DefaultPrimitive::Cube) {}

                    virtual DefaultPrimitive GetType() override { return _type; }

                    void Initialize(_In_ ID3D11Device1* const pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext);

                private:
                    DefaultPrimitive _type;
                };

                ref class SphereMesh sealed
                    : PrimitiveMesh
                {
                internal:
                    SphereMesh() : PrimitiveMesh(), _type(DefaultPrimitive::Sphere) {}

                    virtual DefaultPrimitive GetType() override { return _type; }

                    void Initialize(_In_ ID3D11Device1* const pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext, UINT segments);

                private:
                    DefaultPrimitive _type;
                };

                ref class CylinderMesh sealed
                    : PrimitiveMesh
                {
                internal:
                    CylinderMesh() : PrimitiveMesh(), _type(DefaultPrimitive::Cylinder) {}

                    virtual DefaultPrimitive GetType() override { return _type; }

                    void Initialize(_In_ ID3D11Device1* const pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext, UINT segments);

                private:
                    DefaultPrimitive _type;
                };

                ref class ConeMesh sealed
                    : PrimitiveMesh
                {
                internal:
                    ConeMesh() : PrimitiveMesh(), _type(DefaultPrimitive::Cone) {}

                    virtual DefaultPrimitive GetType() override { return _type; }

                    void Initialize(_In_ ID3D11Device1* const pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext, float baseRadius, UINT segments);

                private:
                    DefaultPrimitive _type;
                };

                ref class PyramidMesh sealed
                    : PrimitiveMesh
                {
                internal:
                    PyramidMesh() : PrimitiveMesh(), _type(DefaultPrimitive::Pyramid) {}

                    virtual DefaultPrimitive GetType() override { return _type; }

                    void Initialize(_In_ ID3D11Device1* const pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext, float baseSize);

                private:
                    DefaultPrimitive _type;
                };

            }
        }
    }
}
