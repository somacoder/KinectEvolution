//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#pragma once

#include <ppltasks.h>
#include <robuffer.h>

using namespace Windows::Storage::Streams;

// Helper utilities for DirectX apps.
namespace DX
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors.
            throw Platform::Exception::CreateException(hr);
        }
    }

    // Converts between Color types.
    inline D2D1_COLOR_F ConvertToColorF(Windows::UI::Color color)
    {
        return D2D1::ColorF(color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f);
    }

    // Converts between Point types.
    inline D2D1_POINT_2F ConvertToPoint2F(Windows::Foundation::Point point)
    {
        return D2D1::Point2F(point.X, point.Y);
    }

    // Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
    inline float ConvertDipsToPixels(float dips)
    {
        static const float dipsPerInch = 96.0f;
        return floorf(dips * Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->LogicalDpi / dipsPerInch + 0.5f); // Round to nearest integer.
    }

    // Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
    inline Windows::Foundation::Point ConvertToScaledPoint(Windows::Foundation::Point point, float dpi)
    {
        static const float dipsPerInch = 96.0f;
        return Windows::Foundation::Point(point.X * dpi / dipsPerInch, point.Y * dpi / dipsPerInch);
    }

    // Function that reads from a binary file asynchronously.
    inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
    {
        using namespace Windows::Storage;
        using namespace Concurrency;

        auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

        return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([](StorageFile^ file)
        {
            return FileIO::ReadBufferAsync(file);
        }).then([](Streams::IBuffer^ fileBuffer) -> std::vector<byte>
        {
            std::vector<byte> returnBuffer;
            returnBuffer.resize(fileBuffer->Length);
            Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
            return returnBuffer;
        });
    }

    inline byte* GetPointerToPixelData(IBuffer^ pixelBuffer)
    {
        // Retrieve the buffer data.
        byte* pixels = nullptr;

        // Query the IBufferByteAccess interface.
        Microsoft::WRL::ComPtr<IBufferByteAccess> bufferByteAccess;
        if (SUCCEEDED(reinterpret_cast<IInspectable*>(pixelBuffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess))))
        {
            bufferByteAccess->Buffer(&pixels);
        }

#pragma warning(suppress: 6102)
        return pixels;
#pragma warning(suppress: 6102)
    }

    class D2DFactoryLock
    {
    public:
        D2DFactoryLock(_In_ ID2D1Factory* d2dFactory)
        {
            DX::ThrowIfFailed(
                d2dFactory->QueryInterface(IID_PPV_ARGS(&m_d2dMultithread))
                );

            m_d2dMultithread->Enter();
        }

        ~D2DFactoryLock()
        {
            m_d2dMultithread->Leave();
        }

    private:
        Microsoft::WRL::ComPtr<ID2D1Multithread> m_d2dMultithread;
    };

#if defined(_DEBUG)
    // Check for SDK Layer support.
    inline bool SdkLayersAvailable()
    {
        HRESULT hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
            0,
            D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
            nullptr,                    // Any feature level will do.
            0,
            D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
            nullptr,                    // No need to keep the D3D device reference.
            nullptr,                    // No need to know the feature level.
            nullptr                     // No need to keep the D3D device context reference.
            );

        return SUCCEEDED(hr);
    }
#endif

}
