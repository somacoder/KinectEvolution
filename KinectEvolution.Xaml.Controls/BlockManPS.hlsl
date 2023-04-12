//------------------------------------------------------------------------------
// <copyright file="BlockManPS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Constant Buffer Variables
cbuffer ConstantBuffer : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProjection;
    float4   lightDir[2];
    float4   lightColor[2];
    float4   outputColor;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 Tex : TEXCOORD0;
};

Texture2D g_txLimb : register(t0);
SamplerState g_sampler : register(s0);

// Pixel shader
float4 main(PS_INPUT input) : SV_Target
{
    float4 finalColor = g_txLimb.Sample(g_sampler, input.Tex);

    // do NdotL lighting for 2 lights
    finalColor += saturate(dot((float3) lightDir[0], input.Normal) * lightColor[0]);
    finalColor += saturate(dot((float3) lightDir[1], input.Normal) * lightColor[1]);

    finalColor.a = 1.0;

    return finalColor;
}
