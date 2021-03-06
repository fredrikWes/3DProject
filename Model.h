#pragma once
#include "Mesh.h"
#include <DirectXMath.h>
#include <d3d11.h>

class Model
{
private:

	Mesh mesh;
	// The World matrix is unique for every object within your world, 
	// and is responsible for transforming the vertices of an object from its own local space, 
	// to a common coordinate system called world space
	XMMATRIX worldMatrix;

	// Water-effect
	XMFLOAT2 movementSpeedUv;
	XMFLOAT2 offsetUV;

	std::string name;

	// Struct containing data of the model
	struct Transform
	{
		XMVECTOR translation;
		XMVECTOR rotation;
		XMVECTOR scale;
		Transform(XMVECTOR translation, XMVECTOR  rotation, XMVECTOR scale) 
			    : translation(translation), rotation(rotation), scale(scale) {}

	}transform;

public:

	Model() = default;
	Model(ID3D11Device* device, std::string name, XMVECTOR position = { 0.0f,0.0f,0.0f }, 
		  XMVECTOR rotation = { 0.0f,0.0f,0.0f }, XMVECTOR scale = { 1.0f, 1.0f, 1.0f });
	~Model();

	// For the game
	bool complete = false;

	// Buffers
	ID3D11Buffer* waterBuffer;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* positionsBuffer;
	
	// Update --> Function to update the models data
	void Update();

	// Getting the vertexCount ( needed for draw function in ModelRenderer.h )
	int GetVertexCount() { return this->mesh.vertexCount; }

	// Get the models worldMatrix -> used in the renderers
	XMMATRIX GetWorldMatrix() { return this->worldMatrix; }

	// Getting the differentBuffers, Mtl and WaterBuffer mostly for for map,unmap and for setting the constant buffers used by the pixel shader pipeline stage
	ID3D11Buffer** GetBuffer() { return &this->vertexBuffer; }
	ID3D11Buffer** GetMTLBuffer() { return &this->mesh.mtlBuffer; }
	ID3D11Buffer** GetWaterBuffer() { return &this->waterBuffer; }
	ID3D11Buffer** GetPositionsBuffer() { return &this->positionsBuffer; }
	XMVECTOR GetPosition() { return this->transform.translation; }
	XMFLOAT2* GetUVOffset() { return &this->offsetUV; }
	const Material::Data& GetMaterial() {return this->mesh.material.data;}

	void BindTextures(ID3D11DeviceContext* context, int startSlot = 0) 
	{ 
		int slot = startSlot; 
		for (auto texture : mesh.material.diffuseTexures) 
		{
			texture->Bind(context, slot); 
			slot++; 
		} 
	}

	void BindDisplacementTexture(ID3D11DeviceContext* context, int startSlot = 0, shaders shader = shaders::DS) 
	{ 
		mesh.material.displacementTexture->Bind(context, startSlot, shader); 
	}

	void AddTexture(ID3D11Device* device, std::string fileName) 
	{ 
		this->mesh.AddDiffuseTexture(device, fileName); 
	}
	
	void SetDisplacementTexture(ID3D11Device* device, std::string path) 
	{ 
		this->mesh.material.displacementTexture = new Texture(device, path); 
	}

	// Setting Tranlation, Rotation and Scale
	void SetTranslation(XMVECTOR translation) {this->transform.translation = translation; }
	void SetRotation(XMVECTOR rotation) { this->transform.rotation = rotation; }
	void SetScale(XMVECTOR scale) {this->transform.scale = scale; }
	XMVECTOR GetRotation() { return this->transform.rotation; }
	// Water
	void WaterSettings(XMFLOAT2 movementSpeedUv, float deltatime);

	void Shutdown()
	{
		mesh.Shutdown();
		vertexBuffer->Release();
		positionsBuffer->Release();
		if (waterBuffer)
			waterBuffer->Release();
	}
};