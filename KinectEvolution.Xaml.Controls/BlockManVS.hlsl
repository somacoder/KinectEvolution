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

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 Tex : TEXCOORD0;
};

// Vertex shader
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    
    float4 pos = mul(mWorld, input.Pos);
    pos = mul(mView, pos);

    output.Pos = mul(mProjection, pos);
    output.Normal = mul((float3x3)mWorld, input.Normal);
    output.Tex = input.Tex;
    
    return output;
}
