//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Texture2D g_Diffuse; // Material albedo for diffuse lighting
Texture2D g_Specular; // Material albedo for specular lighting
Texture2D g_Glow; // Material albedo for glow lighting
float4 g_CameraPosWorld;
matrix g_MeshPass1;
Buffer<float> g_HeightMap;
Texture2D g_NormalMap;


//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
	int g_TerrainRes;
    float4  g_LightDir; // Object space
};

cbuffer cbChangesEveryFrame
{
	matrix g_WorldNormals;
    matrix  g_World;
    matrix  g_WorldViewProjection;
    float   g_Time;
};

cbuffer cbUserChanges
{
};


//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct PosNorTex
{
    float4 Pos : SV_POSITION;
    float4 Nor : NORMAL;
    float2 Tex : TEXCOORD;
};

struct PosTexLi
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
    float   Li : LIGHT_INTENSITY;
	float3 normal: NORMAL;
};

struct PosTex 
{
	float4 Pos : SV_POSITION;
	float2 Tex: TEXCOORD;
};

struct T3dVertexVSIn
{
    float3 Pos : POSITION; //Position in object space
    float2 Tex : TEXCOORD; //Texture coordinate
    float3 Nor : NORMAL; //Normal in object space
    float3 Tan : TANGENT; //Tangent in object space (not used in Ass. 5)
};

struct T3dVertexPSIn
{
    float4 Pos : SV_POSITION; //Position in clip space
    float2 Tex : TEXCOORD; //Texture coordinate
    float3 PosWorld : POSITION; //Position in world space
    float3 NorWorld : NORMAL; //Normal in world space
    float3 TanWorld : TANGENT; //Tangent in world space (not used in Ass. 5)
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

RasterizerState rsDefault {
};

RasterizerState rsCullFront {
    CullMode = Front;
};

RasterizerState rsCullBack {
    CullMode = Back;
};

RasterizerState rsCullNone {
	CullMode = None; 
};

RasterizerState rsLineAA {
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


//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

PosTexLi SimpleVS(PosNorTex Input)
{
    PosTexLi output = (PosTexLi) 0;

    // Transform position from object space to homogenious clip space
    output.Pos = mul(Input.Pos, g_WorldViewProjection);

    // Pass trough normal and texture coordinates
    output.Tex = Input.Tex;

    // Calculate light intensity
    output.normal = normalize(mul(Input.Nor, g_World).xyz); // Assume orthogonal world matrix
    output.Li = saturate(dot(output.normal, g_LightDir.xyz));
        
    return output;
}

float4 SimplePS(PosTexLi Input) : SV_Target0
{
    // Perform lighting in object space, so that we can use the input normal "as it is"
    //float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
    float4 matDiffuse = g_Diffuse.Sample(samLinearClamp, Input.Tex);
    return float4(matDiffuse.rgb * Input.Li, 1);
	//return float4(Input.normal, 1);
}

PosTex TerrainVS(uint VertexID : SV_VertexID)
{
    PosTex output = (PosTex) 0;
    uint x = VertexID % g_TerrainRes;
    uint z = VertexID / g_TerrainRes;
    float px = ((1.0f * x) / (1.0f * g_TerrainRes) - 0.5f);
    float py = g_HeightMap[VertexID];
	float pz = ((1.0f * z) / (1.0f * g_TerrainRes) - 0.5f);
    output.Pos = float4(px, py, pz, 1.0f);
    output.Pos = mul(output.Pos, g_WorldViewProjection);
    float texU = (1.0f * x) / (g_TerrainRes - 1);
    float texV = (1.0f * z) / (g_TerrainRes - 1);
    output.Tex = float2(texU, texV);
    return output;
}

float4 TerrainPS(PosTex input) : SV_Target0
{
    float3 n;
    n.xz = (g_NormalMap.Sample(samAnisotropic, input.Tex).rg * 2.0f).rg - 1.0f;
    n.y = sqrt(1 - n.x * n.x - n.z * n.z);
    n = normalize(mul(float4(n.x, n.y, n.z, 0.0f), g_WorldNormals)).xyz;
    float3 matDiffuse = g_Diffuse.Sample(samLinearClamp, input.Tex).xyz;
    float i = saturate(dot(float4(n.x, n.y, n.z, 0.0f), g_LightDir));
    float3 rgb = matDiffuse * i;
    float4 output = float4(rgb.x, rgb.y, rgb.z, 1.0f);
	return output;
}

T3dVertexPSIn MeshVS(T3dVertexVSIn input)
{
    T3dVertexPSIn output;
    output.Pos = mul(float4(input.Pos.xyz, 1.0f), g_WorldViewProjection);
    output.Tex = input.Tex;
    float4 PosWorld4 = mul(float4(input.Pos.xyz, 1.0f), g_World);
    output.PosWorld = (1.0 / PosWorld4.w) * PosWorld4.xyz;
    output.NorWorld = normalize(mul(float4(input.Nor.xyz, 0.0f), g_WorldNormals).xyz);
    // tangent not needed in this assignment, but is done
    output.TanWorld = normalize(mul(float4(input.Tan.x, input.Tan.y, input.Tan.z, 0.0f), g_World).xyz);
    return output;
}

float4 MeshPS(T3dVertexPSIn input) : SV_Target0
{
    float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, input.Tex);
    float4 matSpecular = g_Specular.Sample(samAnisotropic, input.Tex);
    float4 matGlow = g_Glow.Sample(samAnisotropic, input.Tex);
    float4 colLight = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 colLightAmbient = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float3 n = normalize(input.NorWorld);
    // fixed-direction for now
    float3 i = g_LightDir.xyz;
    float3 r = reflect(-i, n);
    float3 v = normalize((g_CameraPosWorld.xyz - input.PosWorld).xyz);
    float4 output = 0.5f * matDiffuse * saturate(dot(n, i)) * colLight
        + 0.4f * matSpecular * pow(saturate(dot(r, v)), 10) * colLight
        + 0.15f * matDiffuse * colLightAmbient
        + 0.5f * matGlow;
    return output;
    //return g_Diffuse.Sample(samAnisotropic, input.Tex);
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, TerrainPS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }

    pass P1_Mesh
    {
        SetVertexShader(CompileShader(vs_4_0, MeshVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, MeshPS()));
        
        SetRasterizerState(rsCullBack);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
