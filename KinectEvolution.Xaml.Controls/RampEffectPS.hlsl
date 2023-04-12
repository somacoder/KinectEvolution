//------------------------------------------------------------------------------
// <copyright file="RampEffectPS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Constant Buffer Variables
cbuffer PSRampConversionSet : register(b0)
{
    float4 g_rampConversionOutRangeValue;
    float g_rampConversionMultiplier;
    float g_rampConversionRangeBegin;
    float g_rampConversionRangeWrap;
    float g_rampLevels;
};

// texture for ramp conversion
Texture2D rampConversionSourceTexture : register(t0);
Texture2D rampConversionRampTexture : register(t1);

// ramp lookup texture or surface texture for depth
SamplerState textureSampler : register(s0);

struct PS_INPUT
{
    float4 pos    : SV_POSITION;
    float2 tex    : TEXCOORD;
};

// Pixel shader
float4 main(PS_INPUT i) : SV_TARGET
{
    float value = rampConversionSourceTexture.Sample(textureSampler, i.tex).x * g_rampConversionMultiplier;
    if (value < g_rampConversionRangeBegin)
    {
        return g_rampConversionOutRangeValue;
    }

    value -= g_rampConversionRangeBegin;
    if (value > g_rampConversionRangeWrap)
    {
        value = frac(value / g_rampConversionRangeWrap) * g_rampConversionRangeWrap;
    }
    // now value is 0..1, make sure sampling at texel center
    value = value * (g_rampLevels - 1) / g_rampLevels + 0.5f / g_rampLevels;

    return rampConversionRampTexture.Sample(textureSampler, value);
};
