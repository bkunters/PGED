#include "Terrain.h"

#include "GameEffect.h"
#include "SimpleImage.h"
#include <DDSTextureLoader.h>
#include <DirectXMath.h>
#include "DirectXTex.h"

#include "debug.h"

// You can use this macro to access your height field
#define IDX(X,Y,WIDTH) ((X) + (Y) * (WIDTH))

Terrain::Terrain(void):
	indexBuffer(nullptr),
	diffuseTexture(nullptr),
	diffuseTextureSRV(nullptr),
	debugSRV(nullptr)
{
}


Terrain::~Terrain(void)
{
}

HRESULT Terrain::create(ID3D11Device* device, ConfigParser conf)
{
	HRESULT hr;

	// load heightfield
	std::string heightPath = conf.getPathHeight();
	GEDUtils::SimpleImage heightfield(heightPath.c_str());
	int width = heightfield.getWidth();
	int height = heightfield.getHeight();
	std::vector<float> heightValues;
	float max = 0.0f;
	for (int l = 0; l < height; l++)
	{
		for (int i = 0; i < width; i++)
		{
			heightValues.push_back(heightfield.getPixel(i, l));
			if (heightfield.getPixel(i, l) > max)
				max = heightfield.getPixel(i, l);
		}
	}
	maxHeight = max;
	// Note 1: The normal map that you created last week will not be used
	// in this assignment (Assignment 4). It will be of use in later assignments

	// Note 2: For each vertex 10 floats are stored. Do not use another
	// layout.

    // Note 3: In the coordinate system of the vertex buffer (output):
    // x = east,    y = up,    z = south,          x,y,z in [0,1] (float)

	// VertexBuffer Creation
    D3D11_SUBRESOURCE_DATA id;
	// create heightBuffer
	id.pSysMem = &heightValues[0];
	id.SysMemPitch = sizeof(float);
	id.SysMemSlicePitch = 0;

    D3D11_BUFFER_DESC bd;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.ByteWidth = sizeof(float) * heightValues.size();
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;

	V(device->CreateBuffer(&bd, &id, &heightBuffer));
	// create heightBuffer shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvd.Format = DXGI_FORMAT_R32_FLOAT;
	srvd.Buffer.FirstElement = 0;
	srvd.Buffer.NumElements = width * height;
	device->CreateShaderResourceView(heightBuffer, &srvd, &heightBufferSRV);

	std::vector<unsigned int> triangles;
	for (int l = 0; l < height - 1; l++) 
	{
		for (int i = 0; i < width - 1; i++) 
		{
			int leftUpCorner = IDX(i, l, width);
			int rightUpCorner = IDX(i + 1, l, width);
			int leftDownCorner = IDX(i, l + 1, height);
			int rightDownCorner = IDX(i + 1, l + 1, height);
			// first-triangle
			triangles.push_back(leftUpCorner);
			triangles.push_back(rightUpCorner);
			triangles.push_back(leftDownCorner);
			// second-triangle
			triangles.push_back(leftDownCorner);
			triangles.push_back(rightUpCorner);
			triangles.push_back(rightDownCorner);
		}
	}
	indexBufferSize = triangles.size();

	// IndexBufferCreation
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(unsigned int) * triangles.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	// Define initial data
	ZeroMemory(&id, sizeof(id));
	id.pSysMem = &triangles[0];
	// Create Buffer
	V(device->CreateBuffer(&bd, &id, &indexBuffer));

	// in the assignment there is nothing about loading the color texture,
	// so nothing is done for that.
	// load color texture
	std::string tmp = conf.getPathColor();
	const std::wstring wTextPath(tmp.begin(), tmp.end());
	tmp = conf.getPathNormal();
	const std::wstring wNormalPath(tmp.begin(), tmp.end());
	V(DirectX::CreateDDSTextureFromFile(device, wTextPath.c_str(), nullptr, &diffuseTextureSRV));
	V(DirectX::CreateDDSTextureFromFile(device, L"resources\\debug_green.dds", nullptr, &debugSRV));
	V(DirectX::CreateDDSTextureFromFile(device, wNormalPath.c_str(), nullptr, &normalMapSRV));
	if (hr != S_OK) {
		std::string error = "Could not load texture \"" + conf.getPathColor() + "\"";
		MessageBoxA(NULL, error.c_str(), "Invalid texture", MB_ICONERROR | MB_OK);
		return hr;
	}
	g_gameEffect.resolutionEV->SetInt(width);

	// delete[] vertices;
	return hr;
}


void Terrain::destroy()
{
	// SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(debugSRV);
	SAFE_RELEASE(diffuseTextureSRV);
	// diffuse texture is actually not created, but is still released because the assignment says so
	// can be commented, in both ways there are no error
	SAFE_RELEASE(diffuseTexture);
	SAFE_RELEASE(heightBuffer);
	SAFE_RELEASE(heightBufferSRV);
	SAFE_RELEASE(normalMap);
	SAFE_RELEASE(normalMapSRV);
}


void Terrain::render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass)
{
	HRESULT hr;

	// Bind the terrain vertex buffer to the input assembler stage 
    ID3D11Buffer* vbs[] = { nullptr, };
    unsigned int strides[] = { 0, }, offsets[] = { 0, };
    context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);


    // Tell the input assembler stage which primitive topology to use
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);    
	context->IASetInputLayout(nullptr);
	

	V(g_gameEffect.diffuseEV->SetResource(diffuseTextureSRV));
	V(g_gameEffect.heightMapEV->SetResource(heightBufferSRV));
	V(g_gameEffect.normalMapEV->SetResource(normalMapSRV));

    // Apply the rendering pass in order to submit the necessary render state changes to the device
    V(pass->Apply(0, context));

    // Draw
	context->DrawIndexed(indexBufferSize, 0, 0);
}
