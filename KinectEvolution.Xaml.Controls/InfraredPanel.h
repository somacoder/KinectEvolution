//------------------------------------------------------------------------------
// <copyright file="InfraredPanel.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once
#include "Panel.h"
#include "RampEffect.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Infrared {

                using namespace KinectEvolution::Xaml::Controls::Base;

                [Windows::Foundation::Metadata::WebHostHidden]
                public ref class InfraredPanel sealed
                    : public Panel
                {
                public:
                    InfraredPanel();
                    virtual ~InfraredPanel();

                    property WRK::InfraredFrameSource^ InfraredSource
                    {
                        WRK::InfraredFrameSource^ get();
                        void set(WRK::InfraredFrameSource^ value);
                    }

                protected private:
                    virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;
                    void NotifyPropertyChanged(Platform::String^ prop);

                    virtual void Update(double elapsedTime) override;
                    virtual void Render() override;

                    virtual void ResetDeviceResources() override;
                    virtual void CreateDeviceResources() override;
                    virtual void CreateSizeDependentResources() override;

                    // override to add mouse/touch/pen input
                    virtual void StartRenderLoop() override;
                    virtual void StopRenderLoop() override;

                private:
                    void OnInfraredFrame(_In_ WRK::InfraredFrame^ frame);

                private:
                    const UINT IR_FRAME_WIDTH = 512;
                    const UINT IR_FRAME_HEIGHT = 424;

                    // render target view for first stage
                    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      _rampRTV;
                    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      _rampDSV;

                    // shader effect
                    RampEffect^                                         _rampEffect;

                    // texture resources for Kinect frame and ramp
                    Texture^                                            _irTexture;
                    Texture^                                            _irRampTexture;

                    // final output rendered IR image
                    Texture^                                            _irTargetFrame;

                    WRK::InfraredFrameSource^                           _frameSource;
                    WRK::InfraredFrameReader^                           _frameReader;

                };

            }
        }
    }
}
