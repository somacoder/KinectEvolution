//------------------------------------------------------------------------------
// <copyright file="Panel.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "DirectXPanel.h"
#include "StepTimer.h"
#include "Texture.h"
#include "RenderTextureEffect.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Base {

                /// <summary>
                /// extends DirectXPanel to support interactions as well as 
                /// has specific helper functions for rendering textures
                /// </summary>
                [Windows::Foundation::Metadata::WebHostHidden]
                public ref class Panel
                    : public DirectXPanel
                {
                public:
                    void Start() { StartRenderLoop(); }
                    void Stop() { StopRenderLoop(); }

                private protected:
                    Panel();

                    virtual bool IsLoadingComplete() override { return DirectXPanel::IsLoadingComplete() && _loadingComplete; }

                    virtual void ResetDeviceResources() override;
                    virtual void CreateDeviceResources() override;
                    virtual void CreateSizeDependentResources() override;

                    // override to add mouse/touch/pen input
                    virtual void StartRenderLoop() override;
                    virtual void StopRenderLoop() override;

                    // presents the final SRV as the render to the panel
                    void BeginRender(_In_opt_ ID3D11RenderTargetView* pRTV, _In_opt_ ID3D11DepthStencilView* pDSV, _In_opt_ const float* pColor);
                    void RenderTexture(_In_ Texture^ renderTexture);
                    void RenderTextureToSize(_In_ Texture^ renderTexture, UINT targetWidth, UINT targetHeight);
                    void RenderPanel();
                    void EndRender();

                private:
                    // process input events
                    enum class InputState : UINT
                    {
                        None = 0,
                        Tracking,
                        Zooming
                    };

                    void OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
                    void OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
                    void OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);
                    void OnPointerWheelChanged(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e);

                protected private:
                    // final stage render objects
                    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      _backBufferRTV;
                    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>      _backBufferDSV;

                private:
                    BOOL                                                _loadingComplete;

                    // Track our independent input on a background worker thread.
                    Concurrency::critical_section                       _inputLock;

                    // render texture shader
                    RenderTextureEffect^       _renderTextureEffect;

                    Windows::UI::Core::CoreIndependentInputSource^      _coreInput;
                    Windows::UI::Input::GestureRecognizer^              _gestureRecognizer;
                    Windows::Foundation::IAsyncAction^                  _inputLoopWorker;

                    // tracking press on the control
                    InputState                                          _trackState;
                    DirectX::XMFLOAT2                                   _previousPoint[2];
                    UINT                                                _activePointerId[2];

                    float                                               _moveDistance;
                    float                                               _deltaZoomChange;

                    // input event tokens
                    Windows::Foundation::EventRegistrationToken         _pointerPressedToken;
                    Windows::Foundation::EventRegistrationToken         _pointerMovedToken;
                    Windows::Foundation::EventRegistrationToken         _pointerReleasedToken;
                    Windows::Foundation::EventRegistrationToken         _pointerWheelChangedToken;
                };

            }
        }
    }
}
