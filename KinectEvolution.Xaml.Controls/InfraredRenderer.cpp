//------------------------------------------------------------------------------
// <copyright file="InfraredRenderer.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "InfraredRenderer.h"
#include "TextureLock.h"

using namespace KinectEvolution::Xaml::Controls::Base;
using namespace KinectEvolution::Xaml::Controls::DepthMap;

InfraredRenderer::InfraredRenderer()
{
}

InfraredRenderer::~InfraredRenderer()
{
}

void InfraredRenderer::Reset()
{
    _rampRTV.ReleaseAndGetAddressOf();
    _rampDSV.ReleaseAndGetAddressOf();

    _rampEffect = nullptr;
    _irTexture = nullptr;
    _irRampTexture = nullptr;
    _irTargetFrame = nullptr;
}

void InfraredRenderer::Initialize(_In_ ID3D11Device1* pD3DDevice, _In_ ID3D11DeviceContext1* pD3DContext)
{
    _rampEffect = ref new RampEffect();
    _rampEffect->Initialize(pD3DDevice);

    // if everything is loaded, then we can create the texture
    _irTexture = ref new Texture();
    _irTexture->Initialize(pD3DDevice, IR_FRAME_WIDTH, IR_FRAME_HEIGHT, DXGI_FORMAT_R16_UNORM, FALSE);

    // grey scale ramp to use for shader
    _irRampTexture = ref new Texture();
    _irRampTexture->Initialize(pD3DDevice, IR_FRAME_WIDTH, 1, DXGI_FORMAT_R8G8B8A8_UNORM, FALSE);

    UINT rowPitch = 0;
    {
        TextureLock lock(_irRampTexture, pD3DContext);
        UINT* pRamp = static_cast<UINT*>(lock.AccessBuffer(rowPitch));
        if (nullptr != pRamp)
        {
            for (UINT i = 0; i < IR_FRAME_WIDTH; ++i)
            {
                float value = min(1, 1.0f * powf(static_cast<float>(i) / 511, 0.32f));
                UINT greyLevel = static_cast<UINT>(value * 255);
                *pRamp++ = 0xFF000000 | (greyLevel << 16) | (greyLevel << 8) | greyLevel;
            }
        }
    }

    // render target creation
    _irTargetFrame = ref new Texture();
    _irTargetFrame->Initialize(pD3DDevice, IR_FRAME_WIDTH, IR_FRAME_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, TRUE);

    DX::ThrowIfFailed(
        pD3DDevice->CreateRenderTargetView(_irTargetFrame->Texture2D, nullptr, &_rampRTV)
        );

    // Create depth/stencil buffer descriptor.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        static_cast<UINT>(IR_FRAME_WIDTH),
        static_cast<UINT>(IR_FRAME_HEIGHT),
        1,
        1,
        D3D11_BIND_DEPTH_STENCIL);

    // Allocate a 2-D surface as the depth/stencil buffer.
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(
        pD3DDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil)
        );

    // Create depth/stencil view based on depth/stencil buffer.
    const CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc
        = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(
        pD3DDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, &_rampDSV)
        );
}

void InfraredRenderer::UpdateFrameImage(_In_ ID3D11DeviceContext1* pD3DContext, UINT length, _In_count_(length) UINT16* pFrameData)
{
    UINT rowPitch = 0;
    {
        TextureLock lock(_irTexture, pD3DContext);
        BYTE* pDest = static_cast<BYTE*>(lock.AccessBuffer(rowPitch));
        if (nullptr != pDest)
        {
            memcpy(pDest, pFrameData, length);
        }
    }
}

void InfraredRenderer::Render(_In_ ID3D11DeviceContext1* pD3DContext)
{
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> tempRTV;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> tempDSV;

    UINT numViewport = 1;
    D3D11_VIEWPORT vp;

    // get current render target
    pD3DContext->OMGetRenderTargets(1, &tempRTV, &tempDSV);
    pD3DContext->RSGetViewports(&numViewport, &vp);

    // set the render target
    pD3DContext->OMSetRenderTargets(1, _rampRTV.GetAddressOf(), _rampDSV.Get());

    // Clear the back buffer
    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    pD3DContext->ClearRenderTargetView(_rampRTV.Get(), ClearColor);

    // Clear the depth buffer to 1.0 (max depth)
    pD3DContext->ClearDepthStencilView(_rampDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Create and set viewport.
    const D3D11_VIEWPORT txView = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(IR_FRAME_WIDTH), static_cast<float>(IR_FRAME_HEIGHT));

    pD3DContext->RSSetViewports(1, &txView);

    {
        // lock texture(s)
        RenderLock irLock(_irTexture);
        RenderLock irRamplock(_irRampTexture);

        // apply ramp shader
        RampEffect::RampParameter param
            = { XMVectorZero(), 1.0f, 0.0f, FLT_MAX, static_cast<float>(max(_irRampTexture->Width, _irRampTexture->Height)) };

        if (!_rampEffect->Apply(pD3DContext, _irTexture->TextureSRV, _irRampTexture->TextureSRV, &param))
        {
            return;
        }

        // Set primitive topology
        pD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

        // Draw to the texture
        pD3DContext->Draw(1, 0);
    }

    // reset what we did
    ID3D11Buffer* buffer = nullptr;
    pD3DContext->GSSetConstantBuffers(0, 1, &buffer);
    pD3DContext->PSSetConstantBuffers(0, 1, &buffer);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    pD3DContext->PSSetShaderResources(0, 1, &nullSRV);

    // restore previous target
    pD3DContext->OMSetRenderTargets(1, tempRTV.GetAddressOf(), tempDSV.Get());
    pD3DContext->RSSetViewports(numViewport, &vp);
}
