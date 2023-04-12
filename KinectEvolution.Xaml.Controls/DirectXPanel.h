//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#pragma once
#include "pch.h"
#include <concrt.h>
#include "StepTimer.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Base {

                // Base class for a SwapChainPanel-based DirectX rendering surface to be used in XAML apps.
                [Windows::Foundation::Metadata::WebHostHidden]
                public ref class DirectXPanel
                    : public Windows::UI::Xaml::Controls::SwapChainPanel
                {
                internal:
                    DirectXPanel();

                private protected:
                    // children should inherit these
                    virtual void ResetDeviceResources() { };
                    virtual void Update(double elapsedTime) { };
                    virtual void Render() { };

                    virtual void CreateDeviceResources();
                    virtual void CreateSizeDependentResources();

                    virtual void StartRenderLoop();
                    virtual void StopRenderLoop();

                    virtual bool IsLoadingComplete() { return _loadingComplete; }

                protected private:
                    virtual void OnDeviceLost();
                    virtual void OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e);
                    virtual void OnCompositionScaleChanged(Windows::UI::Xaml::Controls::SwapChainPanel ^sender, Platform::Object ^args);
                    virtual void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
                    virtual void OnResuming(Platform::Object^ sender, Platform::Object^ args) { };

                    virtual void Present();

                    void Reset();
                    void CreateDeviceIndependentResources();

                private:
                    property ID3D11Device1* D3D11Device{ ID3D11Device1* get() { return _d3dDevice.Get(); } }
                    property ID3D11DeviceContext1* D3D11Context{ ID3D11DeviceContext1* get() { return _d3dContext.Get(); } }
                    property IDXGISwapChain2* D3D11SwapChain{ IDXGISwapChain2* get() { return _swapChain.Get(); } }

                protected private:
                    Concurrency::critical_section                   _criticalSection;

                    Microsoft::WRL::ComPtr<ID3D11Device1>           _d3dDevice;
                    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    _d3dContext;
                    Microsoft::WRL::ComPtr<IDXGISwapChain2>         _swapChain;

                    D3D_FEATURE_LEVEL                               _featureLevel;

                    Microsoft::WRL::ComPtr<ID2D1Factory2>           _d2dFactory; //todo: make static
                    Microsoft::WRL::ComPtr<ID2D1Device1>            _d2dDevice;
                    Microsoft::WRL::ComPtr<ID2D1DeviceContext1>     _d2dContext;
                    Microsoft::WRL::ComPtr<ID2D1Bitmap1>            _d2dTargetBitmap;

                    Microsoft::WRL::ComPtr<IDXGIOutput>             _dxgiOutput;

                    D2D1_COLOR_F                                    _backgroundColor;
                    DXGI_ALPHA_MODE                                 _alphaMode;

                    // Rendering loop timer.
                    DX::StepTimer                                   _timer;
                    Windows::Foundation::IAsyncAction^              _renderLoopWorker;

                    float                                           _renderTargetHeight;
                    float                                           _renderTargetWidth;

                    float                                           _compositionScaleX;
                    float                                           _compositionScaleY;

                    float                                           _height;
                    float                                           _width;

                private:
                    bool                                            _loadingComplete;
                };
            }
        }
    }
}
