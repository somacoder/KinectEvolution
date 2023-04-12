//------------------------------------------------------------------------------
// <copyright file="Texture.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Base {
                
                struct RenderLock;
                struct TextureLock;
                
                ref class Texture sealed
                {
                public:
                    virtual ~Texture();

                internal:
                    Texture();

                    void Initialize(
                        _In_ ID3D11Device1* pD3DDevice,
                        UINT width, UINT height,
                        DXGI_FORMAT format, BOOL isRenderTarget);

                    property ID3D11Texture2D* Texture2D { ID3D11Texture2D* get() { return _texture2D.Get(); }  }
                    property ID3D11ShaderResourceView* TextureSRV { ID3D11ShaderResourceView* get() { return _textureSRV.Get(); }  }
                    property UINT Width { UINT get() { return _width; }  }
                    property UINT Height { UINT get() { return _height; } }
                    property DXGI_FORMAT TextureFormat { DXGI_FORMAT get() { return _format; } }
                    property D3D11_USAGE TextureUsage { D3D11_USAGE get() { return _usage; } }
                    property BOOL IsRenderTarget { BOOL get() { return _isRenderTarget; } }

                protected private:
                    friend struct TextureLock;
                    friend struct RenderLock;

                    void* Lock(_In_ ID3D11DeviceContext1* const pD3DContext, _Out_ UINT* pRowPitch);
                    void Unlock(_In_ ID3D11DeviceContext1* const pD3DContext);

                    void RenderLock()
                    {
                        if (!_isRenderTarget)
                        {
                            EnterCriticalSection(&_writeLock);
                        }
                    }

                    void RenderUnlock()
                    {
                        if (!_isRenderTarget)
                        {
                            LeaveCriticalSection(&_writeLock);
                        }
                    }

                private:
                    Microsoft::WRL::ComPtr<ID3D11Texture2D>             _stagingTexture;
                    Microsoft::WRL::ComPtr<ID3D11Texture2D>             _texture2D;
                    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    _textureSRV;

                    CRITICAL_SECTION    _writeLock;
                    BOOL                _lockedForWrite;
                    BOOL                _useStaging;

                    UINT            _width;
                    UINT            _height;

                    DXGI_FORMAT     _format;
                    D3D11_USAGE     _usage;

                    BOOL            _isRenderTarget;
                };

            }
        }
    }
}
