//------------------------------------------------------------------------------
// <copyright file="Effect.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Base {

                ref class Mesh abstract
                {
                internal:
                    Mesh() {};

                    void Initialize(_In_ ID3D11Device1* pD3DDevice, UINT numVertices, UINT vertexStride, UINT numIndices, BOOL use16BitIndex);

                    void* LockVertexBuffer(_In_ ID3D11DeviceContext1* pD3DContext);
                    void UnlockVertexBuffer(_In_ ID3D11DeviceContext1* pD3DContext);

                    void* LockIndexBuffer(_In_ ID3D11DeviceContext1* pD3DContext);
                    void UnlockIndexBuffer(_In_ ID3D11DeviceContext1* pD3DContext);

                    void RenderTriangleList(_In_ ID3D11DeviceContext1* pD3DContext, BOOL useIndex);
                    void RenderLineList(_In_ ID3D11DeviceContext1* pD3DContext, BOOL useIndex);
                    void RenderPointList(_In_ ID3D11DeviceContext1* pD3DContext);

                protected private:
                    virtual BOOL IsLoadingComplete() = 0;

                private:
                    void Render(_In_ ID3D11DeviceContext1* pD3DContext, D3D11_PRIMITIVE_TOPOLOGY topology, BOOL useIndex);

                private:
                    Microsoft::WRL::ComPtr<ID3D11Buffer> _vertexBuffer;
                    Microsoft::WRL::ComPtr<ID3D11Buffer> _indexBuffer;

                    UINT _numVertices;
                    UINT _numIndices;
                    UINT _vertexStride;
                    BOOL _use16BitIndex;

                };

            }
        }
    }
}