// VS constants
cbuffer VSTransformSet : register(b0)
{
    float4x4 g_wvp; // world * view * projection
    float4x4 g_wv;  // world * view
};

struct OVERLAY_INPUT
{
    float4 pos    : POSITION;
    float2 tex    : TEXCOORD;
    float4 color  : COLOR;
};

struct OVERLAY_OUTPUT
{
    float4 pos    : SV_POSITION;
    float2 tex    : TEXCOORD;
    float4 color  : COLOR;
};

OVERLAY_OUTPUT main(OVERLAY_INPUT i)
{
    OVERLAY_OUTPUT o;
    o.pos = mul(g_wvp, i.pos);
    o.tex = i.tex;
    o.color = i.color;
    return o;
};
