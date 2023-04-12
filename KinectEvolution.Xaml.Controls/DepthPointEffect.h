//------------------------------------------------------------------------------
// <copyright file="DepthPointEffect.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Effect.h"
#include "Texture.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace DepthMap {

                using namespace KinectEvolution::Xaml::Controls::Base;

                enum class RampMode
                {
                    None = 0,
                    Grey = 1,
                    Color = 2,
                };

                enum class VertexMode
                {
                    Point = 0, // separate XY and Z, with fixed XY. Position calculated in shader so the update is fast
                    Surface = 1, // surface mesh with Position|Tex|Color, more flexible and more feature but update is slow
                    SurfaceWithNormal = 2, // calculate normal for the surface
                    SurfaceWithUV = 3, // use explicit UVs for the surface texture
                    PointXYZ = 4, // the same vertex format as surface, but rendered as POINTLIST
                    PointSprite = 5, // point sprite rendering, requires d3d10 device
                    PointSpriteWithCameraRotation = 6, // point sprite but will rotate with the camera
                };

                ref class DepthPointEffect sealed
                    : Effect
                {
                internal:
                    DepthPointEffect();

                    void Initialize(_In_ ID3D11Device1* const pD3DDevice, UINT width, UINT height, float minZmm, float maxZmm);

                    BOOL Apply(
                        _In_ ID3D11DeviceContext1* const pD3DContext,
                        VertexMode vertexMode, RampMode rampMode,
                        _In_opt_ Texture^ colorTexture,
                        _In_opt_ Texture^ depthTexture);

                    property ID3D11ShaderResourceView* GreyRampSRV { ID3D11ShaderResourceView* get() { return _greyRampTextureSRV.Get(); } }

                    property ID3D11ShaderResourceView* ColorRampSRV { ID3D11ShaderResourceView* get() { return _colorRampTextureSRV.Get(); } }

                protected private:
                    virtual BOOL IsLoadingComplete() override { return _loadingComplete; }

                private:
                    Microsoft::WRL::ComPtr<ID3D11Buffer> _vsConstantBuffer;
                    Microsoft::WRL::ComPtr<ID3D11Buffer> _psConstantBuffer;

                    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _greyRampTextureSRV;  // the default grey ramp texture for depth
                    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _colorRampTextureSRV; // the default color ramp texture for depth

                    UINT _width;
                    UINT _height;
                    float _minZmm;
                    float _maxZmm;

                    BOOL _loadingComplete;
                };

            }
        }
    }
}
