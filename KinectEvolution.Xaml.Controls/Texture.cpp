//------------------------------------------------------------------------------
// <copyright file="Texture.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "Texture.h"

using namespace KinectEvolution::Xaml::Controls::Base;

Texture::Texture()
    : _lockedForWrite(FALSE)
    , _useStaging(FALSE)
{
    InitializeCriticalSectionEx(&_writeLock, 0, 0);
}

Texture::~Texture()
{
    DeleteCriticalSection(&_writeLock);
}

void Texture::Initialize(
    _In_ ID3D11Device1* pD3DDevice,
    UINT width, UINT height,
    DXGI_FORMAT format, BOOL isRenderTarget)
{
    // Create color texture
    D3D11_TEXTURE2D_DESC texDesc = { 0 };
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    if (isRenderTarget)
    {
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    }
    else
    {
        // creating a staging resource to
        // workaround texture format issues with some GPU's
        if (DXGI_FORMAT_G8R8_G8B8_UNORM == format)
        {
            texDesc.BindFlags = 0;
            texDesc.Usage = D3D11_USAGE_STAGING;
            _useStaging = TRUE;
        }
        else
        {
            texDesc.Usage = D3D11_USAGE_DYNAMIC;
        }
        texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        texDesc.MiscFlags = 0;
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
    if (_useStaging)
    {
        DX::ThrowIfFailed(
            pD3DDevice->CreateTexture2D(&texDesc, nullptr, &stagingTexture)
            );

        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
   }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    DX::ThrowIfFailed(
        pD3DDevice->CreateTexture2D(&texDesc, nullptr, &texture)
        );

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    DX::ThrowIfFailed(
        pD3DDevice->CreateShaderResourceView(texture.Get(), nullptr, &srv)
        );

    // set locals
    _width = width;
    _height = height;
    _format = format;
    _isRenderTarget = isRenderTarget;
    _usage = texDesc.Usage;

    if (_useStaging)
    {
        stagingTexture.As(&_stagingTexture);
    }

    texture.As(&_texture2D);
    srv.As(&_textureSRV);
}

#pragma warning(suppress: 26115)
#pragma warning(suppress: 6101)
void* Texture::Lock(_In_ ID3D11DeviceContext1* const pD3DContext, _Out_ UINT* pRowPitch)
{
    if (_isRenderTarget)
    {
        *pRowPitch = 0;

        return nullptr; // cannot lock texture created as render target
    }

    EnterCriticalSection(&_writeLock);

    HRESULT hr = S_OK;
    D3D11_MAPPED_SUBRESOURCE map;
    if (_useStaging)
    {
        hr = pD3DContext->Map(_stagingTexture.Get(), 0, D3D11_MAP_WRITE, 0, &map);
    }
    else
    {
        hr = pD3DContext->Map(_texture2D.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    }

    if (FAILED(hr))
    {
        LeaveCriticalSection(&_writeLock);
        return nullptr;
    }

    *pRowPitch = map.RowPitch;
    _lockedForWrite = TRUE;

    return map.pData;
}

void Texture::Unlock(_In_ ID3D11DeviceContext1* const pD3DContext)
{
    if (_isRenderTarget || !_lockedForWrite)
    {
        return; // cannot unlock texture created as render target
    }

    if (_useStaging)
    {
        pD3DContext->Unmap(_stagingTexture.Get(), 0);

        pD3DContext->CopyResource(_texture2D.Get(), _stagingTexture.Get());
    }
    else
    {
        pD3DContext->Unmap(_texture2D.Get(), 0);
    }
    
    LeaveCriticalSection(&_writeLock);
    _lockedForWrite = FALSE;
}
