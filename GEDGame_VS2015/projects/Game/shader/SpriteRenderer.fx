//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Texture2DArray g_Textures[50];

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbChangesEveryFrame
{
    matrix g_ViewProjection;
    float4 g_cameraUp;
    float4 g_cameraRight;
};

//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct SpriteVertex
{
    float3 Pos : POSITION;
    float Rad : RADIUS;
    int TexIndex : TEXTUREINDEX;
    float t : TIME;
    float alpha : ALPHA;
};

struct PSVertex
{
    float4 Pos : SV_POSITION;
    int TexIndex : TEXTUREINDEX;
    float2 Tex : TEXCOORD;
    float t : TIME;
    float alpha : ALPHA;
};

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

RasterizerState rsDefault
{
};

RasterizerState rsCullFront
{
    CullMode = Front;
};

RasterizerState rsCullBack
{
    CullMode = Back;
};

RasterizerState rsCullNone
{
    CullMode = None;
};

RasterizerState rsLineAA
{
    CullMode = None;
    AntialiasedLineEnable = true;
};


//--------------------------------------------------------------------------------------
// DepthStates
//--------------------------------------------------------------------------------------
DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

BlendState BSBlendOver
{
    BlendEnable[0] = TRUE;
    SrcBlend[0] = SRC_ALPHA;
    SrcBlendAlpha[0] = ONE;
    DestBlend[0] = INV_SRC_ALPHA;
    DestBlendAlpha[0] = INV_SRC_ALPHA;
};

//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

void SpriteVS(inout SpriteVertex vertex) {}

[maxvertexcount(4)]
void SpriteGS(point SpriteVertex vertex[1], inout TriangleStream<PSVertex> stream)
{
    float4 p1, p2, p3, p4;
    p1 = mul(float4(vertex[0].Pos + vertex[0].Rad * g_cameraRight.xyz + vertex[0].Rad * g_cameraUp.xyz, 1.0f), g_ViewProjection);
    p2 = mul(float4(vertex[0].Pos + vertex[0].Rad * g_cameraRight.xyz - vertex[0].Rad * g_cameraUp.xyz, 1.0f), g_ViewProjection);
    p3 = mul(float4(vertex[0].Pos - vertex[0].Rad * g_cameraRight.xyz + vertex[0].Rad * g_cameraUp.xyz, 1.0f), g_ViewProjection);
    p4 = mul(float4(vertex[0].Pos - vertex[0].Rad * g_cameraRight.xyz - vertex[0].Rad * g_cameraUp.xyz, 1.0f), g_ViewProjection);
    PSVertex v1 = { p1, vertex[0].TexIndex, float2(0.0f, 0.0f), vertex[0].t, vertex[0].alpha };
    PSVertex v2 = { p2, vertex[0].TexIndex, float2(1.0f, 0.0f), vertex[0].t, vertex[0].alpha };
    PSVertex v3 = { p3, vertex[0].TexIndex, float2(0.0f, 1.0f), vertex[0].t, vertex[0].alpha };
    PSVertex v4 = { p4, vertex[0].TexIndex, float2(1.0f, 1.0f), vertex[0].t, vertex[0].alpha };
    stream.Append(v1);
    stream.Append(v2);
    stream.Append(v3);
    stream.Append(v4);
}

float4 SpritePS(PSVertex input) : SV_Target0
{
    float4 output = (float4) 0;
    float3 dimensions;
    switch (input.TexIndex)
    {
        case 0:
            g_Textures[0].GetDimensions(dimensions.x, dimensions.y, dimensions.z);
            output = g_Textures[0].Sample(samAnisotropic, float3(input.Tex, input.t * dimensions.z));
            break;
        case 1:
            g_Textures[1].GetDimensions(dimensions.x, dimensions.y, dimensions.z);
            output = g_Textures[1].Sample(samAnisotropic, float3(input.Tex, input.t * dimensions.z));
            break;
        case 2:
            g_Textures[2].GetDimensions(dimensions.x, dimensions.y, dimensions.z);
            output = g_Textures[2].Sample(samAnisotropic, float3(input.Tex, input.t * dimensions.z));
            break;
    } 
    output = float4(output.rgb, output.a * input.alpha);
    return output;
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------

technique11 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, SpriteVS()));
        SetGeometryShader(CompileShader(gs_4_0, SpriteGS()));
        SetPixelShader(CompileShader(ps_4_0, SpritePS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
