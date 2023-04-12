//------------------------------------------------------------------------------
// <copyright file="InfraredRenderer.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "RampEffect.h"
#include "Texture.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace DepthMap {

                using namespace KinectEvolution::Xaml::Controls::Base;

                ref class InfraredRenderer sealed
                {

                internal:
                    InfraredRenderer();

                    void Initialize(_In_ ID3D11Device1* pD3DDevice, _In_ ID3D11DeviceContext1* pD3DContext);

                    void Reset();

                    void UpdateFrameImage(_In_ ID3D11DeviceContext1* pD3DContext, UINT length, _In_count_(length) UINT16* pFrameData);

                    void Render(_In_ ID3D11DeviceContext1* pD3DContext);

                    property Texture^ InfraredImage { Texture^ get() { return _irTargetFrame; } }

                private:
                    ~InfraredRenderer();

                private:
                    const UINT IR_FRAME_WIDTH = 512;
                    const UINT IR_FRAME_HEIGHT = 424;

                    // render target view for first stage
                    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  _rampRTV;
                    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  _rampDSV;

                    // shader effect
                    RampEffect^            _rampEffect;

                    // texture resources for Kinect frame and ramp
                    Texture^               _irTexture;
                    Texture^               _irRampTexture;

                    // final output rendered IR image
                    Texture^               _irTargetFrame;

                };

            }
        }
    }
}
