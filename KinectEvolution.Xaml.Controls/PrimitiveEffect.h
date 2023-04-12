//------------------------------------------------------------------------------
// <copyright file="PrimitiveEffect.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once
#include "Effect.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Base {

                struct MatrixViewConstants
                {
                    // CONST 0-3    world * view * projection
                    XMMATRIX _wvp;

                    // CONST 4-7    world * view
                    XMMATRIX _wv;
                };

                struct PrimitiveParameter
                {
                    // view space light
                    XMVECTOR _ambient;
                    XMVECTOR _diffuse;
                    XMVECTOR _specular;
                    XMVECTOR _direction;
                    float _power;
                    BOOL _enableLighting;

                    // texture
                    BOOL _enableTexture;

                    // multiply color
                    BOOL _enableMultiplyColor;
                    XMVECTOR _multiplyColor;
                };

                ref class PrimitiveEffect sealed
                    : Effect
                {
                internal:
                    PrimitiveEffect();

                    void Initialize(_In_ ID3D11Device1* const pd3dDevice);

                    BOOL Apply(_In_ ID3D11DeviceContext1* const pd3dContext, _In_opt_ const PrimitiveParameter* pParam);

                    virtual BOOL IsLoadingComplete() override { return _loadingComplete; }

                    void ApplyTransformAll(_In_ ID3D11DeviceContext1* const pD3DContext, _In_ CXMMATRIX worldViewMatrix, _In_ CXMMATRIX worldViewProjectionMatrix);

                    // utility function for calculate unit up vector transform matrix
                    static DirectX::XMMATRIX TransformUnitUpVector(_In_ CXMVECTOR pStartPosition, _In_ CXMVECTOR pEndPosition, BOOL noStretch);

                protected private:
                    Microsoft::WRL::ComPtr<ID3D11Buffer>    _lightConstantBuffer;
                    Microsoft::WRL::ComPtr<ID3D11Buffer>    _cameraConstantBuffer;

                    BOOL _loadingComplete;
                };

            }
        }
    }
}
