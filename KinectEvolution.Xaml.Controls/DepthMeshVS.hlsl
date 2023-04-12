//------------------------------------------------------------------------------
// <copyright file="DepthMeshVS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "shaders.h"

// Constant Buffer Variables
cbuffer VSTransformSet : register(b0)
{
    float4x4 g_wvp; // world * view * projection
    float4x4 g_wv;  // world * view
};

cbuffer VSSensorInfoSet : register(b1)
{
    float4 g_sensorSize; // w, h, min_z(in mm), max_z(in mm)
    float3 g_textureSize; // w, h, vertex mode
};

// vertex shader textures
SamplerState vertexTextureSampler : register(s0);
Texture2D<float>  vertexDepthTexture : register(t0);
Texture2D<float2> vertexXYTexture : register(t1);
Texture2D<float2> vertexUVTexture : register(t2);

struct VS_INPUT
{
    float2 xy : POSITION;
};

struct GS_INPUT
{
    float4 pos      : SV_POSITION;
    float4 texcoord : TEXCOORD0; // xy for surface texture, z for ramp texture
    float2 stexcoord : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float3 view   : TEXCOORD3;
};

float4 GetPos(float2 depthTexcoord)
{
    float zmm = (float)vertexDepthTexture.SampleLevel(vertexTextureSampler, depthTexcoord, 0) * 65535;

    if (zmm >= g_sensorSize.z &&
        zmm <= g_sensorSize.w)
    {
        float z = zmm / 1000.0f; // convert to meters.
        float2 xy = vertexXYTexture.SampleLevel(vertexTextureSampler, depthTexcoord, 0);

        return float4(xy * z, z, 1.0f);
    }
    else
    {
        return 0;
    }
}

float3 CalcNormal(float4 pos0, float4 pos1, float4 pos2)
{
    return cross(pos1.xyz - pos0.xyz, pos2.xyz - pos0.xyz);
}

GS_INPUT main(VS_INPUT i)
{
    GS_INPUT o;

    float2 depthTexcoord = (i.xy + 0.5) / g_sensorSize.xy;
    float4 pos = GetPos(depthTexcoord);

    o.pos = mul(g_wvp, pos);

    float rampCoord = frac((pos.z - 0.5) / 4.0); // let color ramp wrap in 4 meters

    o.texcoord = float4(depthTexcoord, rampCoord, 1);

    float vertexMode = g_textureSize.z;

    if (vertexMode == VERTEX_MODE_SURFACE_WITH_UV)
    {
        float2 uv = vertexUVTexture.SampleLevel(vertexTextureSampler, depthTexcoord, 0);
        o.stexcoord = (uv + 0.5) / g_textureSize.xy;
    }
    else
    {
        o.stexcoord = 0;
    }

    if ((vertexMode == VERTEX_MODE_SURFACE_WITH_NORMAL || vertexMode == VERTEX_MODE_POINTSPRITE_WITH_NORMAL) &&
        pos.z != 0)
    {
        float4 pos_top = GetPos(depthTexcoord - float2(0, 1) / g_sensorSize.xy);
        float4 pos_bottom = GetPos(depthTexcoord + float2(0, 1) / g_sensorSize.xy);
        float4 pos_left = GetPos(depthTexcoord - float2(1, 0) / g_sensorSize.xy);
        float4 pos_right = GetPos(depthTexcoord + float2(1, 0) / g_sensorSize.xy);

        float3 normal = CalcNormal(pos, pos_left, pos_top);
        normal += CalcNormal(pos, pos_top, pos_right);
        normal += CalcNormal(pos, pos_right, pos_bottom);
        normal += CalcNormal(pos, pos_bottom, pos_left);
        o.normal = mul(g_wv, float4(normalize(normal), 0)).xyz;

        float4 eye = float4(0, 0, 0, 1);
        float4 viewPos = mul(g_wv, pos);
        o.view = normalize((eye - viewPos).xyz);
    }
    else
    {
        o.normal = 0;
        o.view = 0;
    }

    return o;
};
