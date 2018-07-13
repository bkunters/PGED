#pragma once
#include "DXUT.h"
#include "d3dx11effect.h"
#include "ConfigParser.h"

struct Vertex 
{
	DirectX::XMFLOAT4 Pos; // Position
	DirectX::XMFLOAT4 Normal; // Normal
	DirectX::XMFLOAT2 UV; // Texture coordinates
};


class Terrain
{
public:
	Terrain(void);
	~Terrain(void);

	// added a ConfigParser object for the config
	HRESULT create(ID3D11Device* device, ConfigParser conf);
	void destroy();
	void render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass);
	float getMaxHeight() { return maxHeight; };

private:
	Terrain(const Terrain&);
	Terrain(const Terrain&&);
	void operator=(const Terrain&);

	// Terrain rendering resources
	// ID3D11Buffer*                           vertexBuffer;	// The terrain's vertices
	ID3D11Buffer*                           indexBuffer;	// The terrain's triangulation
	UINT									indexBufferSize; // the size of the indexBuffer
	ID3D11Texture2D*                        diffuseTexture; // The terrain's material color for diffuse lighting
	ID3D11ShaderResourceView*               diffuseTextureSRV; // Describes the structure of the diffuse texture to the shader stages
	ID3D11Texture2D*						normalMap;
	ID3D11ShaderResourceView*				normalMapSRV;
	ID3D11Buffer*							heightBuffer;
	ID3D11ShaderResourceView*				heightBufferSRV;

	// a maxHeight for camera
	float maxHeight;

	// General resources
	ID3D11ShaderResourceView*               debugSRV;
};

