//------------------------------------------------------------------------------
// <copyright file="BlockManEffect.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Effect.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace BlockMan {

                using namespace KinectEvolution::Xaml::Controls::Base;

                ref class BlockManEffect sealed
                    : Effect
                {
                internal:
                    struct BlockManParams
                    {
                        DirectX::XMMATRIX mWorld;
                        DirectX::XMMATRIX mView;
                        DirectX::XMMATRIX mProjection;
                        DirectX::XMFLOAT4 lightDir[2];
                        DirectX::XMFLOAT4 lightColor[2];
                        DirectX::XMFLOAT4 outputColor;
                    };

                internal:
                    BlockManEffect();

                    void Initialize(_In_ ID3D11Device1* const pD3DDevice);

                    BOOL Apply(_In_ ID3D11DeviceContext1* const pD3DDeviceContext, _In_ BlockManEffect::BlockManParams* pParams);

                protected private:
                    virtual BOOL IsLoadingComplete() override { return _loadingComplete; }

                private:

                    // shader constant buffer
                    Microsoft::WRL::ComPtr<ID3D11Buffer>                _constantBuffer;

                    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    _textureSRV;

                    Microsoft::WRL::ComPtr<ID3D11SamplerState>          _textureSampler;

                    BOOL _loadingComplete;
                };

            }
        }
    }
}