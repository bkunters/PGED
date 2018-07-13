#pragma once

#include "DXUT.h"
#include "d3dx11effect.h"
#include "SDKmisc.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>

// Convenience macros for safe effect variable retrieval
#define SAFE_GET_PASS(Technique, name, var)   {assert(Technique!=NULL); var = Technique->GetPassByName( name );						assert(var->IsValid());}
#define SAFE_GET_TECHNIQUE(effect, name, var) {assert(effect!=NULL); var = effect->GetTechniqueByName( name );						assert(var->IsValid());}
#define SAFE_GET_SCALAR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsScalar();			assert(var->IsValid());}
#define SAFE_GET_VECTOR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsVector();			assert(var->IsValid());}
#define SAFE_GET_MATRIX(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsMatrix();			assert(var->IsValid());}
#define SAFE_GET_SAMPLER(effect, name, var)   {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsSampler();			assert(var->IsValid());}
#define SAFE_GET_RESOURCE(effect, name, var)  {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsShaderResource();	assert(var->IsValid());}


struct GameEffect
{
	// A D3DX rendering effect
	ID3DX11Effect*                          effect; // The whole rendering effect
	ID3DX11EffectTechnique*                 technique; // One technique to render the effect
	ID3DX11EffectPass*                      pass0; // One rendering pass of the technique
	ID3DX11EffectMatrixVariable*            worldEV; // World matrix effect variable
	ID3DX11EffectMatrixVariable*            worldViewProjectionEV; // WorldViewProjection matrix effect variable
	ID3DX11EffectShaderResourceVariable*    diffuseEV; // Effect variable for the diffuse color texture
	ID3DX11EffectVectorVariable*            lightDirEV; // Light direction in object space
	ID3DX11EffectShaderResourceVariable*	heightMapEV; // heightmap that will bind to effect
	ID3DX11EffectShaderResourceVariable*	normalMapEV; // normalmap that will bind to effect
	ID3DX11EffectScalarVariable*			resolutionEV; // resolution of the heightmap
	ID3DX11EffectMatrixVariable*			worldNormalsEV; // WorldNormals matrix effect variable
	ID3DX11EffectShaderResourceVariable*	specularEV; // Effect variable for the specular color texture;
	ID3DX11EffectShaderResourceVariable*	glowEV; // Effect variable for the glow color texture
	ID3DX11EffectVectorVariable*			cameraPosWorldEV; // Effect variable for the camera position (in world)
	ID3DX11EffectPass*						meshPass1; // Pass effect for the mesh

	GameEffect() { ZeroMemory(this, sizeof(*this)); }		// WARNING: This will set ALL members to 0!


	HRESULT create(ID3D11Device* device)
	{
		HRESULT hr;
		WCHAR path[MAX_PATH];

		// Find and load the rendering effect
		V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"shader\\game.fxo"));
		std::ifstream is(path, std::ios_base::binary);
		is.seekg(0, std::ios_base::end);
		std::streampos pos = is.tellg();
		is.seekg(0, std::ios_base::beg);
		std::vector<char> effectBuffer((unsigned int)pos);
		is.read(&effectBuffer[0], pos);
		is.close();
		V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, device, &effect));    
		assert(effect->IsValid());

		// Obtain the effect technique
		SAFE_GET_TECHNIQUE(effect, "Render", technique);

		// Obtain the effect pass
		SAFE_GET_PASS(technique, "P0", pass0);

		// Obtain the effect variables
		SAFE_GET_RESOURCE(effect, "g_Diffuse", diffuseEV);
		SAFE_GET_MATRIX(effect, "g_World", worldEV);
		SAFE_GET_MATRIX(effect, "g_WorldViewProjection", worldViewProjectionEV);   
		SAFE_GET_VECTOR(effect, "g_LightDir", lightDirEV);

		// obtain specular, glow, meshpass and camera-position
		SAFE_GET_RESOURCE(effect, "g_Specular", specularEV);
		SAFE_GET_RESOURCE(effect, "g_Glow", glowEV);
		SAFE_GET_PASS(technique, "P1_Mesh", meshPass1);
		SAFE_GET_VECTOR(effect, "g_CameraPosWorld", cameraPosWorldEV);

		// Obtain heigtMap, normalMap, resolution and worldNormals
		SAFE_GET_RESOURCE(effect, "g_HeightMap", heightMapEV);
		SAFE_GET_RESOURCE(effect, "g_NormalMap", normalMapEV);
		SAFE_GET_SCALAR(effect, "g_TerrainRes", resolutionEV);
		SAFE_GET_MATRIX(effect, "g_WorldNormals", worldNormalsEV);

		return S_OK;
	}


	void destroy()
	{
		SAFE_RELEASE(effect);
	}
};

extern GameEffect g_gameEffect;