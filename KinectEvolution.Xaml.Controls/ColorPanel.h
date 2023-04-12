//------------------------------------------------------------------------------
// <copyright file="ColorPanel.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Panel.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Color {

                using namespace KinectEvolution::Xaml::Controls::Base;

                [Windows::Foundation::Metadata::WebHostHidden]
                public ref class ColorPanel sealed
                    : public Panel
                {
                public:
                    ColorPanel();

                    property WRK::ColorFrameSource^ ColorSource
                    {
                        WRK::ColorFrameSource^ get();
                        void set(WRK::ColorFrameSource^ value);
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
                    ~ColorPanel();

                    void OnColorFrame(_In_ WRK::ColorFrame^ frame);

                private:
                    const UINT COLOR_FRAME_WIDTH = 1920;
                    const UINT COLOR_FRAME_HEIGHT = 1080;

                    // color frame image
                    Texture^                            _colorFrame;

                    WRK::ColorFrameSource^              _frameSource;
                    WRK::ColorFrameReader^              _frameReader;
                };

            }
        }
    }
}
