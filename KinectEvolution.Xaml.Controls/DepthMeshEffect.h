//------------------------------------------------------------------------------
// <copyright file="DepthMeshEffect.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Texture.h"
#include "DepthPointEffect.h"
#include "PrimitiveEffect.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace DepthMap {

                using namespace KinectEvolution::Xaml::Controls::Base;

                ref class DepthMeshEffect sealed
                    : Effect
                {
                internal:
                    DepthMeshEffect();

                    void Initialize(_In_ ID3D11Device1* const pD3DDevice, UINT width, UINT height, float minZmm, float maxZmm);

                    BOOL Apply(
                        _In_ ID3D11DeviceContext1* const pD3DContext,
                        VertexMode vertexMode,
                        RampMode rampMode,
                        _In_ const XMMATRIX& view,
                        _In_ const XMMATRIX& proj,
                        _In_opt_ const PrimitiveParameter* pEffect,
                        _In_opt_ Texture^ pTexture,
                        _In_opt_ DepthPointEffect^ pDepthPointEffect,
                        _In_opt_ Texture^ pDepthTexture,
                        _In_opt_ Texture^ pXYTexture,
                        _In_opt_ Texture^ pUVTexture);

                    void ApplyTransformAll(_In_ ID3D11DeviceContext1* const pD3DContext, _In_ const XMMATRIX& worldViewMatrix, _In_ const XMMATRIX&  worldViewProjectionMatrix);

                protected private:
                    virtual BOOL IsLoadingComplete() override { return _loadingComplete; }

                private:
                    Microsoft::WRL::ComPtr<ID3D11GeometryShader>    _geometryShader;
                    Microsoft::WRL::ComPtr<ID3D11Buffer>            _vsConstantBuffer;
                    Microsoft::WRL::ComPtr<ID3D11Buffer>            _gsConstantBuffer;
                    Microsoft::WRL::ComPtr<ID3D11Buffer>            _psConstantBuffer;
                    Microsoft::WRL::ComPtr<ID3D11SamplerState>      _vsSampler;
                    Microsoft::WRL::ComPtr<ID3D11Buffer>            _cameraConstantBuffer;

                    UINT _width;
                    UINT _height;
                    float _minZmm;
                    float _maxZmm;
                    float _pixelSizemm;

                    BOOL _loadingComplete;
                };

            }
        }
    }
}
