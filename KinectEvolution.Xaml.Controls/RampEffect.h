//------------------------------------------------------------------------------
// <copyright file="RampEffect.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once
#include "Effect.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Base {

                ref class RampEffect sealed
                    : Effect
                {
                internal:
                    struct RampParameter
                    {
                        XMVECTOR _outRangeValue;
                        float _multiplier;
                        float _rangeBegin;
                        float _rangeWrap;
                        float _rampLevels;
                    };

                    RampEffect();

                    void Initialize(_In_ ID3D11Device1* const pD3DDevice);

                    BOOL Apply(
                        _In_ ID3D11DeviceContext1* const pD3DContext,
                        _In_ ID3D11ShaderResourceView* pIRSRV,
                        _In_ ID3D11ShaderResourceView* pIRRampSRV,
                        _In_opt_ const RampParameter* pParam);

                protected private:
                    virtual BOOL IsLoadingComplete() override { return _loadingComplete; }

                private:
                    // geometry shader
                    Microsoft::WRL::ComPtr<ID3D11GeometryShader>    _geometryShader;

                    // shader constant buffer
                    Microsoft::WRL::ComPtr<ID3D11Buffer>            _constantBuffer;

                    // texture sampler for pixel shader
                    Microsoft::WRL::ComPtr<ID3D11SamplerState>      _textureSampler;

                    bool _loadingComplete;
                };

            }
        }
    }
}
