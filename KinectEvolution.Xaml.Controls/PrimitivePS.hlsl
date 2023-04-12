//------------------------------------------------------------------------------
// <copyright file="PrimitivePS.hlsl" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Constant Buffer Variables
cbuffer PSMeshSet : register(b0)
{
    float4 g_misc; //x: enable lighting, y: not used, z: specular power, w: ambient only lighting
    float4 g_ambient;
    float4 g_diffuse;
    float4 g_specular;
    float4 g_lightDir;
};

struct PS_INPUT
{
    float4 pos    : SV_POSITION;
    float3 normal : TEXCOORD;
    float3 view   : TEXCOORD1;
    float4 color  : COLOR;
};

// Pixel shader
float4 main(PS_INPUT i) : SV_Target
{
    float4 color = i.color;

    if (g_misc.x > 0)
    {
        color = i.color * g_ambient;

        if (g_misc.w == 0)
        {
            float3 N = normalize(i.normal);
            float3 V = normalize(i.view);
            float3 L = -g_lightDir.xyz;
            float3 H = normalize(L + V);
            float  d = max(dot(N, L), 0);
            float  s = max(dot(N, H), 0);

            color += float4(g_diffuse.rgb * d + g_specular.rgb * pow(s, g_misc.z), 0);
        }
    }

    return color;
};
