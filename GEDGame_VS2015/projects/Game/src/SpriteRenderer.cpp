#include "SpriteRenderer.h"
#include "DXUT.h"
#include "d3dx11effect.h"
#include "SDKmisc.h"
#include <DDSTextureLoader.h>

#include <iostream>
#include <fstream>
#include <sstream>

// Convenience macros for safe effect variable retrieval
#define SAFE_GET_PASS(Technique, name, var)   {assert(Technique!=NULL); var = Technique->GetPassByName( name );						assert(var->IsValid());}
#define SAFE_GET_TECHNIQUE(effect, name, var) {assert(effect!=NULL); var = effect->GetTechniqueByName( name );						assert(var->IsValid());}
#define SAFE_GET_SCALAR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsScalar();			assert(var->IsValid());}
#define SAFE_GET_VECTOR(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsVector();			assert(var->IsValid());}
#define SAFE_GET_MATRIX(effect, name, var)    {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsMatrix();			assert(var->IsValid());}
#define SAFE_GET_SAMPLER(effect, name, var)   {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsSampler();			assert(var->IsValid());}
#define SAFE_GET_RESOURCE(effect, name, var)  {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsShaderResource();	assert(var->IsValid());}

SpriteRenderer::SpriteRenderer(const std::vector<std::wstring>& textureFilenames) 
{
	m_textureFilenames = textureFilenames;
	m_pEffect = nullptr;
	m_spriteSRV = std::vector<ID3D11ShaderResourceView*>();
	m_spriteCountMax = 1024;
	m_pVertexBuffer = nullptr;
	m_pInputLayout = nullptr;
}

SpriteRenderer::~SpriteRenderer() {}

HRESULT SpriteRenderer::reloadShader(ID3D11Device* pDevice) 
{
	HRESULT hr;
	WCHAR path[MAX_PATH];

	// Find and load the rendering effect
	V_RETURN(DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"shader\\SpriteRenderer.fxo"));
	std::ifstream is(path, std::ios_base::binary);
	is.seekg(0, std::ios_base::end);
	std::streampos pos = is.tellg();
	is.seekg(0, std::ios_base::beg);
	std::vector<char> effectBuffer((unsigned int)pos);
	is.read(&effectBuffer[0], pos);
	is.close();
	V_RETURN(D3DX11CreateEffectFromMemory((const void*)&effectBuffer[0], effectBuffer.size(), 0, pDevice, &m_pEffect));
	assert(m_pEffect->IsValid());

	// Obtain the effect technique
	SAFE_GET_TECHNIQUE(m_pEffect, "Render", technique);
	// Obtain the effect pass
	SAFE_GET_PASS(technique, "P0", pass);
	// Obtain the ViewProjection matrix
	SAFE_GET_MATRIX(m_pEffect, "g_ViewProjection", viewProjectionEV);
	// Obtain the right and up vectors of the camera
	SAFE_GET_VECTOR(m_pEffect, "g_cameraUp", cameraUpEV);
	SAFE_GET_VECTOR(m_pEffect, "g_cameraRight", cameraRightEV);

	return S_OK;
}

void SpriteRenderer::releaseShader() 
{
	SAFE_RELEASE(m_pEffect);
}

HRESULT SpriteRenderer::create(ID3D11Device* pDevice) 
{
	HRESULT hr;

	// create the texture resource views
	for (int i = 0; i < m_textureFilenames.size(); i++) 
	{
		ID3D11ShaderResourceView* textureSRV;
		V(DirectX::CreateDDSTextureFromFile(pDevice, m_textureFilenames[i].c_str(), nullptr, &textureSRV));
		m_spriteSRV.push_back(textureSRV);
	}

	// create vertexBuffer
	D3D11_BUFFER_DESC bd;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = sizeof(SpriteVertex) * m_spriteCountMax;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	V_RETURN(pDevice->CreateBuffer(&bd, nullptr, &m_pVertexBuffer));

	// Define Input Layout
	const D3D11_INPUT_ELEMENT_DESC layout[] = // http://msdn.microsoft.com/en-us/library/bb205117%28v=vs.85%29.aspx
	{
		{ "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "RADIUS",    0, DXGI_FORMAT_R32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTUREINDEX",      0, DXGI_FORMAT_R32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TIME",  0, DXGI_FORMAT_R32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ALPHA",     0, DXGI_FORMAT_R32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	D3DX11_PASS_DESC passDesc;
	pass->GetDesc(&passDesc);

	// Input Layout Creation
	V_RETURN(pDevice->CreateInputLayout(layout, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout));

	return S_OK;
}

void SpriteRenderer::destroy() 
{
	SAFE_RELEASE(m_pEffect)
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);
	for (int i = 0; i < m_spriteSRV.size(); i++) 
	{
		SAFE_RELEASE(m_spriteSRV[i]);
	}
	m_spriteSRV.clear();
}

void SpriteRenderer::renderSprites(ID3D11DeviceContext* context, const std::vector<SpriteVertex>& sprites, const CFirstPersonCamera& camera) 
{
	HRESULT hr;

	D3D11_BOX box;
	box.left = 0; box.right = sprites.size() * sizeof(SpriteVertex);
	box.top = 0; box.bottom = 1;
	box.front = 0; box.back = 1;

	unsigned int strides[] = { sizeof(SpriteVertex), }, offsets[] = { 0, };

	context->UpdateSubresource(m_pVertexBuffer, 0, &box, &sprites[0], 0, 0);
	context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, strides, offsets);
	context->IASetInputLayout(m_pInputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// Set ViewProjection matrix, right and up vector of the camera
	DirectX::XMMATRIX g_ViewProjection = camera.GetViewMatrix() * camera.GetProjMatrix();
	V(viewProjectionEV->SetMatrix((float*)&g_ViewProjection));
	V(cameraUpEV->SetFloatVector((float*)&camera.GetWorldRight()));
	V(cameraRightEV->SetFloatVector((float*)&camera.GetWorldUp()));

	// Set textures for plasma and gatling sprites
	m_pEffect->GetVariableByName("g_Textures")->AsShaderResource()->SetResourceArray(&(m_spriteSRV[0]), 0, m_spriteSRV.size());
	
	pass->Apply(0, context);
	context->Draw(sprites.size(), 0);
}