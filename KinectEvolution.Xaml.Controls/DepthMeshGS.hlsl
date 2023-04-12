//------------------------------------------------------------------------------
// <copyright file="BlockManPS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "shaders.h"

// Constant Buffer Variables
cbuffer GSDepthPointSet : register(b0)
{
    float4x4 g_proj;                     // projection
    float4 g_worldSpacePixelWidthHeight; // x: pixel size(in mm) yzw: unused
};

struct PS_INPUT // GS_INPUT
{
    float4 pos      : POSITION;
    float4 texcoord : TEXCOORD0; // xy for surface texture, z for ramp texture
    float2 stexcoord : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float3 view   : TEXCOORD3;
};

// Geometry shader
[maxvertexcount(4)]
void main(point PS_INPUT input[1], inout TriangleStream<PS_INPUT> OutputStream)
{
    PS_INPUT output = (PS_INPUT) 0;

    // the vertex shader removed this point because the z value was out of range
    // skip generating extra verticies here
    // not skipping here causes some graphics card drivers to crash.
    if (input[0].pos.x == 0 && input[0].pos.y == 0)
        return;

    float4 x_Offset[4] =
    { { -1.0f, -1.0f, 0.0f, 0.0f },
    { -1.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f, -1.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f, 0.0f },
    };
    float sizeInMeters = g_worldSpacePixelWidthHeight.x / 1000.0f;

    output.texcoord = input[0].texcoord;
    output.stexcoord = input[0].stexcoord;
    //TODO: calculate the normal for each new vertex according to the new pos
    output.normal = input[0].normal;
    output.view = input[0].view;

    for (uint i = 0; i<4; i += 1)
    {
        output.pos = mul(g_proj, input[0].pos + x_Offset[i] * sizeInMeters);
        OutputStream.Append(output);
    }
    OutputStream.RestartStrip();
}
