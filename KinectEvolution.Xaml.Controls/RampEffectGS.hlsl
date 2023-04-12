//------------------------------------------------------------------------------
// <copyright file="RampEffectGS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Constant Buffer Variables
struct GS_INPUT
{
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

// vertex offsets for building a quad from a depth pixel
static const float4 quadOffsets[4] =
{
    float4(-1.0, 1.0, 0.0, 1.0),
    float4(1.0, 1.0, 0.0, 1.0),
    float4(-1.0, -1.0, 0.0, 1.0),
    float4(1.0, -1.0, 0.0, 1.0)
};

static const float2 texCoords[4] =
{
    float2(0.0, 0.0),
    float2(1.0, 0.0),
    float2(0.0, 1.0),
    float2(1.0, 1.0)
};

// Takes in a single vertex point.  Expands it into the 4 vertices of a quad.
[maxvertexcount(4)]
void main(point GS_INPUT particles[1], uint primID : SV_PrimitiveID, inout TriangleStream<PS_INPUT> triStream)
{
    PS_INPUT output;

    [unroll]
    for (int c = 0; c < 4; ++c)
    {
        output.pos = float4(quadOffsets[c].xy, 0.0, 1.0);
        output.tex = texCoords[c];

        // add this vertex to the triangle strip
        triStream.Append(output);
    }
}
