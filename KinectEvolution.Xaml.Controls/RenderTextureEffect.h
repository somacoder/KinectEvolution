//------------------------------------------------------------------------------
// <copyright file="RenderTextureEffect.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Effect.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Base {

                ref class RenderTextureEffect sealed :
                    Effect
                {
                internal:
                    struct Paramaters
                    {
                        DirectX::XMFLOAT4 texParams; // multiple of 16
                        DirectX::XMFLOAT2 texSize;
                        DirectX::XMFLOAT2 ctlSize;
                        DirectX::XMFLOAT4 color;
                    };

                    RenderTextureEffect();

                    void Initialize(_In_ ID3D11Device1* const pD3DDevice);

                    BOOL Apply(
                        _In_ ID3D11DeviceContext1* const pD3DContext,
                        _In_ ID3D11ShaderResourceView* pTexture,
                        _In_ const RenderTextureEffect::Paramaters* pParam);

                protected private:
                    virtual BOOL IsLoadingComplete() override { return _loadingComplete; }

                private:
                    // geometry shader
                    Microsoft::WRL::ComPtr<ID3D11GeometryShader>    _geometryShader;

                    // shader constant buffer
                    Microsoft::WRL::ComPtr<ID3D11Buffer>            _constantBuffer;

                    // texture sampler for pixel shader
                    Microsoft::WRL::ComPtr<ID3D11SamplerState>      _textureSampler;

                    // rasterizer state
                    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   _rasterState;

                    bool _loadingComplete;
                };

            }
        }
    }
}
