//------------------------------------------------------------------------------
// <copyright file="DepthPointPS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "shaders.h"

// Constant Buffer Variables
cbuffer PSDepthPointSet : register(b0)
{
    float4 g_norampcolor;
    float4 g_sensorInfo;  // 1/w, 1/h, min_z(in mm), max_z(in mm)
    float  g_vertexMode;
    float  g_rampMode;
    float  g_useSurfaceTexture;
    float  g_enablemultiplycolor;  // It happens that this and the above made it 32 bits aligned.
    float4 g_multiplycolor;
}

// *****************************************************************************
// Color space conversion
// *****************************************************************************
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

// ramp lookup texture or surface texture for depth
SamplerState textureSampler : register(s0);
Texture2D depthRampTexture : register(t0);
Texture2D depthTexture : register(t1);

struct PS_INPUT
{
    float4 pos      : SV_POSITION;
    float4 texcoord : TEXCOORD0; // xy for surface texture, z for ramp texture
    float2 stexcoord : TEXCOORD1;
};

// Pixel Shader
float4 main(PS_INPUT i) : SV_TARGET
{
    float4 output = 0;
    float shouldClip = 0;
    if (g_vertexMode == VERTEX_MODE_SURFACE 
        || g_vertexMode == VERTEX_MODE_SURFACE_WITH_NORMAL 
        || g_vertexMode == VERTEX_MODE_SURFACE_WITH_UV) // needs to kill pixels when rendering surface
    {
        // check depth difference of neighboring pixels
        float2 gridTexcoord = i.texcoord.xy / g_sensorInfo.xy;
        gridTexcoord = floor(gridTexcoord - 0.5f) + 0.5f;
        float2 normalizedGridTexcoord = gridTexcoord * g_sensorInfo.xy;
        float z = depthTexture.Sample(textureSampler, normalizedGridTexcoord).x * 65535;
        float z0 = depthTexture.Sample(textureSampler, normalizedGridTexcoord + float2(g_sensorInfo.x, 0)).x * 65535;
        float z1 = depthTexture.Sample(textureSampler, normalizedGridTexcoord + float2(g_sensorInfo.x, g_sensorInfo.y)).x * 65535;
        float z2 = depthTexture.Sample(textureSampler, normalizedGridTexcoord + float2(0, g_sensorInfo.y)).x * 65535;
        float maxZ = max(max(z, z0), max(z1, z2));
        float minZ = min(min(z, z0), min(z1, z2));
        shouldClip = maxZ - minZ > DEPTH_TRIANGLE_THRESHOLD_MM || z < g_sensorInfo.z || z > g_sensorInfo.w;
    }

    if (g_rampMode == RAMP_MODE_NONE)
    {
        if (g_useSurfaceTexture != SURFACE_TEXTURE_MODE_NONE)
        {
            float2 texcoord = i.texcoord.xy;

            // using uv table
            if (g_vertexMode == VERTEX_MODE_SURFACE_WITH_UV)
            {
                texcoord = i.stexcoord.xy;
            }

            float4 sample = depthRampTexture.Sample(textureSampler, texcoord); // surface texture

            // yuv color space
            if (g_useSurfaceTexture == SURFACE_TEXTURE_MODE_YUV)
            {
                sample = yuy2_to_rgb(sample);
            }

            output = sample;
        }
        else
        {
            output = g_norampcolor;
        }
    }
    else
    {
        output = depthRampTexture.Sample(textureSampler, i.texcoord.z); // ramp texture
    }

    clip(shouldClip > 0 ? -1 : 1);

    if (g_enablemultiplycolor > 0.0)
    {
        output = output * g_multiplycolor;
    }

    return output;
}
