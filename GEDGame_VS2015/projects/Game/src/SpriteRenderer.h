#pragma once

#include <string>
#include <vector>

#include <DXUT.h>
#include <DXUTcamera.h>

#include <d3dx11effect.h>

struct SpriteVertex
{
	DirectX::XMFLOAT3 position;     // world-space position (sprite center)
	float radius;                   // world-space radius (= half side length of the sprite quad)
	int textureIndex;               // which texture to use (out of SpriteRenderer::m_spriteSRV)
	float t;
	float alpha;
};

struct GunProjectileVertex 
{
	SpriteVertex projectile;		// projectile itself
	float lifeTime;					// how long is this sprite alive
	float maxLifeTime;				// how much is this sprite allowed to live
	DirectX::XMFLOAT3 velocity;		// velocity of projectile
};

struct ExplosionState 
{
	DirectX::XMFLOAT3 position;
	int textureIndex;
	float timePassed;
	float size;
};

struct ParticleState 
{
	SpriteVertex particle;
	DirectX::XMFLOAT3 velocity;
};

class SpriteRenderer
{
public:
	// Constructor: Create a SpriteRenderer with the given list of textures.
	// The textures are *not* be created immediately, but only when create is called!
	SpriteRenderer(const std::vector<std::wstring>& textureFilenames);
	// Destructor does nothing. Destroy and ReleaseShader must be called first!
	~SpriteRenderer();

	// Load/reload the effect. Must be called once before create!
	HRESULT reloadShader(ID3D11Device* pDevice);
	// Release the effect again.
	void releaseShader();

	// Create all required D3D resources (textures, buffers, ...).
	// reloadShader must be called first!
	HRESULT create(ID3D11Device* pDevice);
	// Release D3D resources again.
	void destroy();

	// Render the given sprites. They must already be sorted into back-to-front order.
	void renderSprites(ID3D11DeviceContext* context, const std::vector<SpriteVertex>& sprites, const CFirstPersonCamera& camera);

private:
	std::vector<std::wstring> m_textureFilenames;

	// Rendering effect (shaders and related GPU state). Created/released in Reload/ReleaseShader.
	ID3DX11Effect* m_pEffect;

	ID3DX11EffectTechnique* technique; // One technique to render the effect
	ID3DX11EffectPass* pass; // One rendering pass of the technique
	ID3DX11EffectMatrixVariable* viewProjectionEV; // Effect variable for the ViewProjection matrix
	ID3DX11EffectVectorVariable* cameraRightEV; // Effect variable for the up vector of the camera
	ID3DX11EffectVectorVariable* cameraUpEV; // Effect variable for the right vector of the camera

	// Sprite textures and corresponding shader resource views.
	//std::vector<ID3D11Texture2D*>          m_spriteTex;       // You may not need this if you use CreateDDSTExtureFromFile!
	std::vector<ID3D11ShaderResourceView*> m_spriteSRV;

	// Maximum number of allowed sprites, i.e. size of the vertex buffer.
	size_t m_spriteCountMax;
	// Vertex buffer for sprite vertices, and corresponding input layout.
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11InputLayout* m_pInputLayout;
};
