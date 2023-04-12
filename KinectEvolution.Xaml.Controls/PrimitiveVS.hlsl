//------------------------------------------------------------------------------
// <copyright file="PrimitiveVS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Constant Buffer Variables
cbuffer VSTransformSet : register(b0)
{
    float4x4 g_wvp; // world * view * projection
    float4x4 g_wv;  // world * view
};

struct VS_INPUT
{
    float3 pos    : POSITION;
    float3 normal : NORMAL;
    float4 color  : COLOR;
};

struct PS_INPUT
{
    float4 pos    : SV_POSITION;
    float3 normal : TEXCOORD;
    float3 view   : TEXCOORD1;
    float4 color  : COLOR;
};

// Vertex Shader
PS_INPUT main(VS_INPUT i)
{
    PS_INPUT o;

    o.pos = mul(g_wvp, float4(i.pos, 1.0f));
    
    float4 viewPos = mul(g_wv, float4(i.pos, 1.0f));
    
    o.normal = mul(g_wv, float4(i.normal, 0.0f)).xyz;
    
    float4 eye = float4(0.0f, 0.0f, 0.0f, 1.0f);
    o.view = (eye - viewPos).xyz;

    o.color = i.color;
    
    return o;
};
