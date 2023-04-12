//------------------------------------------------------------------------------
// <copyright file="RenderTexturePS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "shaders.h"

// Constant Buffer Variables
cbuffer cb : register(b0)
{
    float4 surfaceTextureMode;
    float2 texSize;
    float2 ctlSize;
    float4 color;
}

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 color : COLOR;
};

Texture2D<float4> txImage : register(t0);
sampler txSampler;

// Color space conversion
float4 yuy2_to_rgb(float4 yuv)
{
    float4 output;

    clip(yuv.g == 0.0f ? -1 : 1);

    output.r = clamp(yuv.g + 1.568648f * (yuv.b - 0.501961f), 0.0f, 1.0f);
    output.g = clamp(yuv.g - 0.186593f * (yuv.r - 0.501961f) - 0.466296f * (yuv.b - 0.501961f), 0.0f, 1.0f);
    output.b = clamp(yuv.g + 1.848352f * (yuv.r - 0.501961f), 0.0f, 1.0f);
    output.a = 1.0f;

    return output;
}

// Pixel Shader
float4 main(PS_INPUT input) : SV_Target
{
    float4 baseColor = input.color;
    float2 texcoord = input.tex.xy;
    float4 texColor = txImage.Sample(txSampler, texcoord); // surface texture

    if (surfaceTextureMode.x == SURFACE_TEXTURE_MODE_YUV)
    {
        texColor = yuy2_to_rgb(texColor);
        baseColor *= texColor;
    }

    clip(baseColor.a == 0 ? -1 : 1);

    return texColor;
}
