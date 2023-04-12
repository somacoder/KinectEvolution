//------------------------------------------------------------------------------
// <copyright file="BlockManMesh.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "BlockManEffect.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace BlockMan {

                using namespace KinectEvolution::Xaml::Controls::Base;

                ref class BlockManMesh sealed
                    : Mesh
                {
                internal:
                    BlockManMesh();

                    void Initialize(_In_ ID3D11Device1* pD3DDevice, _In_ ID3D11DeviceContext1* const pD3DContext);

                    void Render(_In_ ID3D11DeviceContext1* pD3DContext, BOOL useIndex, _In_ BlockManEffect::BlockManParams* pParams);

                    BOOL IsLoadingComplete() override { return _loadingComplete; }

                private:
                    struct BlockManVertex
                    {
                        DirectX::XMFLOAT3 pos;
                        DirectX::XMFLOAT3 normal;
                        DirectX::XMFLOAT2 tex;
                    };

                private:

                    BlockManEffect^ _blockManFX;

                    BOOL _loadingComplete;
                };

            }
        }
    }
}
