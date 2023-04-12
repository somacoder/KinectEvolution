//------------------------------------------------------------------------------
// <copyright file="ColorPanel.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "ColorPanel.h"
#include "DirectXHelper.h"
#include "TextureLock.h"

using namespace KinectEvolution::Xaml::Controls::Base;
using namespace KinectEvolution::Xaml::Controls::Color;

using namespace Concurrency;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;

using namespace WindowsPreview::Kinect;

ColorPanel::ColorPanel()
    : Panel()
{
    critical_section::scoped_lock lock(_criticalSection);

    CreateDeviceResources();
    CreateSizeDependentResources();
}

ColorPanel::~ColorPanel()
{
    Stop();
}

void ColorPanel::StartRenderLoop()
{
    DirectXPanel::StartRenderLoop();
}

void ColorPanel::StopRenderLoop()
{
    DirectXPanel::StopRenderLoop();
}

/// <summary>
/// Sets the color source for this control
/// </summary>
void ColorPanel::ColorSource::set(ColorFrameSource^ value)
{
    if (_frameSource == value)
    {
        return;
    }

    // close the previous reader
    _frameReader = nullptr;

    // set the new source
    _frameSource = value;
    _frameReader = _frameSource->OpenReader();

    NotifyPropertyChanged("ColorSource");
}

ColorFrameSource^ ColorPanel::ColorSource::get()
{
    return _frameSource;
}

void ColorPanel::NotifyPropertyChanged(Platform::String^ prop)
{
    PropertyChangedEventArgs^ args = ref new PropertyChangedEventArgs(prop);
    PropertyChanged(this, args);
}

void ColorPanel::Update(double elapsedTime)
{
    if (nullptr != _frameReader)
    {
        ColorFrame^ frame = _frameReader->AcquireLatestFrame();
        if (nullptr != frame)
        {
            OnColorFrame(frame);
        }
    }
}

void ColorPanel::Render()
{
    if (!IsLoadingComplete())
    {
        return;
    }

    // Set render targets to the screen.
    BeginRender(nullptr, nullptr, nullptr);

    // bind viewport the size of the control
    const D3D11_VIEWPORT backBufferView = CD3D11_VIEWPORT(0.0f, 0.0f, _renderTargetWidth, _renderTargetHeight);
    _d3dContext->RSSetViewports(1, &backBufferView);

    {
        RenderLock lock(_colorFrame);

        RenderTexture(_colorFrame);
    }

    EndRender();
}

void ColorPanel::OnColorFrame(_In_ ColorFrame^ frame)
{
    UINT length = frame->FrameDescription->LengthInPixels * frame->FrameDescription->BytesPerPixel;

    IBuffer^ buffer = frame->LockRawImageBuffer();

    UINT16* pSrc = reinterpret_cast<UINT16*>(DX::GetPointerToPixelData(buffer));

    // Map the texture to PS shader resource texture
    {
        TextureLock lock(_colorFrame, _d3dContext.Get());

        UINT rowPitch = 0;
        UINT16* pDest = static_cast<UINT16*>(lock.AccessBuffer(rowPitch));
        if (nullptr != pDest)
        {
            memcpy(pDest, pSrc, length); // yuy2
        }
    }
}

void ColorPanel::ResetDeviceResources()
{
    Panel::ResetDeviceResources();

    _colorFrame = nullptr;
}

void ColorPanel::CreateDeviceResources()
{
    Panel::CreateDeviceResources();

    _colorFrame = ref new Texture();
    _colorFrame->Initialize(
        _d3dDevice.Get(),
        COLOR_FRAME_WIDTH,
        COLOR_FRAME_HEIGHT,
        DXGI_FORMAT_G8R8_G8B8_UNORM,
        FALSE);

    Render(); // render one frame to force resize
}

void ColorPanel::CreateSizeDependentResources()
{
    Panel::CreateSizeDependentResources();

    Render();
}
