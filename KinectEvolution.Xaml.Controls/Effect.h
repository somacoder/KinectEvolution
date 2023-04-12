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

                // basic default structure for shader
                struct EffectConstant
                {
                    // view space light
                    XMVECTOR    _ambient;
                    XMVECTOR    _diffuse;
                    XMVECTOR    _specular;
                    XMVECTOR    _direction;
                    float       _power;
                    BOOL        _enableLighting;

                    // texture
                    BOOL        _enableTexture;

                    // multiply color
                    BOOL        _enableMultiplyColor;
                    XMVECTOR    _multiplyColor;
                };

                ref class Effect abstract
                {
                internal:
                    Effect();

                protected private:
                    void InitializeVS(
                        _In_ ID3D11Device1* const pD3DDevice,
                        _In_reads_bytes_(vertexShaderByteCodeLength) const void* pVertexShaderByteCode,
                        _In_ SIZE_T vertexShaderByteCodeLength,
                        _In_reads_(numInputElementDescs) const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs,
                        _In_ UINT numInputElementDescs);

                    void InitializePS(
                        _In_ ID3D11Device1* const pD3DDevice,
                        _In_reads_bytes_(pixelShaderByteCodeLength) const void* pPixelShaderByteCode,
                        _In_ SIZE_T pixelShaderByteCodeLength);

                    // must be called by base classes to setup VS / PS Shader
                    BOOL Apply(_In_ ID3D11DeviceContext1* const pD3DContext);

                    virtual BOOL IsLoadingComplete() = 0;

                protected private:
                    Microsoft::WRL::ComPtr<ID3D11VertexShader>  _vertexShader;
                    Microsoft::WRL::ComPtr<ID3D11PixelShader>   _pixelShader;
                    Microsoft::WRL::ComPtr<ID3D11InputLayout>   _inputLayout;

                };

            }
        }
    }
}