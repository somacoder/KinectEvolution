//------------------------------------------------------------------------------
// <copyright file="TextureLock.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "pch.h"
#include "Texture.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Base {

                // lock mechanism to use when using Texture to render
                struct RenderLock
                {
                private:
                    Texture^    _texture = nullptr;

                public:
                    RenderLock(_In_ Texture^ texture)
                        : _texture(texture)
                    {
                        if (nullptr != _texture)
                        {
                            texture->RenderLock();
                        }
                    }

                    ~RenderLock()
                    {
                        if (nullptr != _texture)
                        {
                            _texture->RenderUnlock();
                        }
                    }

                };

                // lock mechanism to use with Texture object
                struct TextureLock
                {
                private:
                    Microsoft::WRL::ComPtr <ID3D11DeviceContext1> _d3dContext;
                    Texture^    _texture = nullptr;
                    void*       _pData = nullptr;
                    UINT        _rowPitch = 0;

                public:
                    TextureLock(_In_ Texture^ texture, _In_ ID3D11DeviceContext1* pD3DContext)
                        : _texture(texture)
                        , _d3dContext(pD3DContext)
                        , _rowPitch(0)
                        , _pData(nullptr)
                    {
                        if (nullptr != _texture && nullptr != _d3dContext)
                        {
                            _pData = _texture->Lock(_d3dContext.Get(), &_rowPitch);
                        }
                    }

                    ~TextureLock()
                    {
                        if (nullptr != _texture && nullptr != _d3dContext)
                        {
                            _texture->Unlock(_d3dContext.Get());
                        }
                    }

                    void* AccessBuffer(_Inout_ UINT& rowPitch)
                    {
                        rowPitch = _rowPitch;

                        return _pData;
                    }

                };

            }
        }
    }
}