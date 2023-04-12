//------------------------------------------------------------------------------
// <copyright file="DepthMeshPS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "shaders.h"

// PS constants
cbuffer PSDepthMeshSet : register(b0)
{
    float4 g_norampcolor;
    float4 g_sensorInfo;  // 1/w, 1/h, min_z(in mm), max_z(in mm)
    float4 g_renderModes; // vertexMode, rampMode, useSurfaceTexture

    float4 g_misc; //x: enable lighting, y: enable texturing, z: specular power, w: ambient only lighting
    float4 g_ambient;
    float4 g_diffuse;
    float4 g_specular;
    float4 g_lightDir;

    float4 g_enableMultiplyColor;    // use float4 to make it 32 bits aligned.
    float4 g_multiplyColor;
}

// pixel shader textures
SamplerState textureSampler : register(s0);
Texture2D depthRampTexture : register(t0);
Texture2D depthTexture : register(t1);

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

struct PS_INPUT
{
    float4 pos      : SV_POSITION;
    float4 texcoord : TEXCOORD0; // xy for surface texture, z for ramp texture
    float2 stexcoord : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float3 view   : TEXCOORD3;
};

// Pixel shader
float4 main(PS_INPUT i) : SV_TARGET
{
    float4 output = 0;
    float shouldClip = 0;
    float vertexMode = g_renderModes.x;
    float rampMode = g_renderModes.y;
    float useSurfaceTexture = g_renderModes.z;

    if (vertexMode == VERTEX_MODE_SURFACE ||
        vertexMode == VERTEX_MODE_SURFACE_WITH_NORMAL ||
        vertexMode == VERTEX_MODE_SURFACE_WITH_UV) // needs to kill pixels when rendering surface
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

    clip(shouldClip > 0 ? -1 : 1);

    if (rampMode == RAMP_MODE_NONE)
    {
        output = float4(1, 1, 1, 1);

        if (vertexMode == VERTEX_MODE_SURFACE_WITH_NORMAL || vertexMode == VERTEX_MODE_POINTSPRITE_WITH_NORMAL)
        {
            if (g_misc.x > 0)
            {
                output = g_ambient;
                if (g_misc.w == 0)
                {
                    float3 N = normalize(i.normal);
                    float3 V = normalize(i.view);
                    float3 L = -g_lightDir.xyz;
                    float3 H = normalize(L + V);
                    float  d = max(dot(N, L), 0);
                    float  s = max(dot(N, H), 0);
                    output += float4(g_diffuse.rgb * d + g_specular.rgb * pow(s, g_misc.z), 0);
                }
            }
        }

        if (useSurfaceTexture != SURFACE_TEXTURE_MODE_NONE && g_misc.y > 0)
        {
            float2 texcoord = i.texcoord.xy;

            // using uv table
            if (vertexMode == VERTEX_MODE_SURFACE_WITH_UV)
            {
                texcoord = i.stexcoord.xy;
            }

            float4 sample = depthRampTexture.Sample(textureSampler, texcoord); // surface texture

            // yuv color space
            if (useSurfaceTexture == SURFACE_TEXTURE_MODE_YUV)
            {
                sample = yuy2_to_rgb(sample);
            }

            output *= sample;
        }
        else
        {
            output *= g_norampcolor;
        }
    }
    else
    {
        output = depthRampTexture.Sample(textureSampler, i.texcoord.z); // ramp texture
    }

    if (g_enableMultiplyColor.x > 0.0)
    {
        output = output * g_multiplyColor;
    }

    return output;
}
