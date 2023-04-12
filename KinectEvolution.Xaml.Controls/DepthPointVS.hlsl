//------------------------------------------------------------------------------
// <copyright file="DepthPointVS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Constant Buffer Variables
cbuffer VSTransformSet : register(b0)
{
    float4x4 g_wvp; // world * view * projection
    float4x4 g_wv;  // world * view
};

cbuffer VSSensorInfoSet : register(b1)
{
    float4 g_sensorSize; // w, h, min_z(in mm), max_z(in mm)
    float2 g_textureSize; // w, h
};

struct VS_INPUT
{
    float2 xy : POSITION0;
    float  z : POSITION1;
    float2 uv : TEXCOORD0;
    float2 suv : TEXCOORD1;
};

struct PS_INPUT
{
    float4 pos      : SV_POSITION;
    float4 texcoord : TEXCOORD0; // xy for surface texture, z for ramp texture
    float2 stexcoord : TEXCOORD1;
};

// Vertex shader
PS_INPUT main(VS_INPUT i)
{
    PS_INPUT o;

    if (i.z > g_sensorSize.w || i.z < g_sensorSize.z)
    {
        o.pos = 0;
        o.texcoord = 0;
        o.stexcoord = 0;
    }
    else
    {
        float z = i.z / 1000.0f; // convert to meters.

        float4 pos = float4(i.xy * z, z, 1.0f);

        o.pos = mul(g_wvp, pos);

        float rampCoord = frac((z - 0.5) / 4.0); // let color ramp wrap in 4 meters

        // texel from (0.5, 0.5) to (w-0.5, h-0.5),
        // need the 0.5 shift to make sure sampling at texel center
        o.texcoord = float4((i.uv + 0.5) / g_sensorSize.xy, rampCoord, 1);

        o.stexcoord = (i.suv + 0.5) / g_textureSize.xy;
    }

    return o;
};
