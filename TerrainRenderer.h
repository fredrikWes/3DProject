#pragma once
#include "Model.h"
#include "D3D11Helper.h"
#include "ShaderData.h"
#include <fstream>

class TerrainRenderer
{
private:
	// Struct in Geometry
	const unsigned int stride = sizeof(Vertex);
	const unsigned int offset = 0;
	//Deferred
	const std::string vs_path = "x64/Debug/TerrainVertexShader.cso";
	ID3D11VertexShader* vertexShader;
	const std::string hs_path = "x64/Debug/TerrainHullShader.cso";
	ID3D11HullShader* hullShader;
	const std::string ds_path = "x64/Debug/TerrainDomainShader.cso";
	ID3D11DomainShader* domainShader;
	const std::string ps_path = "x64/Debug/TerrainPixelShader.cso";
	ID3D11PixelShader* pixelShader;
	const std::string gs_path = "x64/Debug/TerrainGeometryShader.cso";
	ID3D11GeometryShader* geometryShader;

	ID3D11Buffer* matricesBuffer;
	ID3D11Buffer* lightBuffer;

	struct Matrices
	{ 
		XMFLOAT4X4 viewPerspective; 
		XMFLOAT4X4 worldSpace; 
	}matrices;
public:

	TerrainRenderer(ID3D11Device* device) :matrices(), hullShader(nullptr), domainShader(nullptr), vertexShader(nullptr), geometryShader(nullptr), pixelShader(nullptr)
	{
		CreateBuffer(device, matricesBuffer, sizeof(Matrices));
		CreateBuffer(device, lightBuffer, sizeof(XMMATRIX));

		std::string shaderData;
		std::ifstream reader;
		std::string shaderByteCode;

		// HULL SHADER
		reader.open(hs_path, std::ios::binary | std::ios::beg);

		if (!reader.is_open())
		{
			std::cout << "COULD NOT OPEN FILE: " + hs_path << std::endl;
			return;
		}

		reader.seekg(0, std::ios::end);
		shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
		reader.seekg(0, std::ios::beg);
		shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());

		HRESULT hr = device->CreateHullShader(shaderData.c_str(), shaderData.length(), nullptr, &hullShader);
		if FAILED(hr)
		{
			std::cout << "FAILED TO CREATE HULL SHADER" << std::endl;
			return;
		}

		shaderData.clear();
		reader.close();

		// PIXEL SHADER
		reader.open(ps_path, std::ios::binary | std::ios::beg);

		if (!reader.is_open())
		{
			std::cout << "COULD NOT OPEN FILE: " + ps_path << std::endl;
			return;
		}

		reader.seekg(0, std::ios::end);
		shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
		reader.seekg(0, std::ios::beg);
		shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());

		hr = device->CreatePixelShader(shaderData.c_str(), shaderData.length(), nullptr, &pixelShader);
		if FAILED(hr)
		{
			std::cout << "FAILED TO CREATE PIXEL SHADER" << std::endl;
			return;
		}

		shaderData.clear();
		reader.close();

		// DOMAIN SHADER
		reader.open(ds_path, std::ios::binary | std::ios::beg);

		if (!reader.is_open())
		{
			std::cout << "COULD NOT OPEN FILE: " + ds_path << std::endl;
			return;
		}

		reader.seekg(0, std::ios::end);
		shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
		reader.seekg(0, std::ios::beg);
		shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());

		hr = device->CreateDomainShader(shaderData.c_str(), shaderData.length(), nullptr, &domainShader);
		if FAILED(hr)
		{
			std::cout << "FAILED TO CREATE DOMAIN SHADER" << std::endl;
			return;
		}
		shaderData.clear();
		reader.close();

		// VERTEX SHADER
		reader.open(vs_path, std::ios::binary | std::ios::ate);
		if (!reader.is_open())
		{
			std::cerr << "Could not open VS file!" << std::endl;
			return;
		}

		reader.seekg(0, std::ios::end);
		shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
		reader.seekg(0, std::ios::beg);

		shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());

		if (FAILED(device->CreateVertexShader(shaderData.c_str(), shaderData.length(), nullptr, &vertexShader)))
		{
			std::cerr << "FAILED TO CREATE VERTEX SHADER!" << std::endl;
			return;
		}

		shaderData.clear();
		reader.close();

		// GEOMETRY SHADER
		reader.open(gs_path, std::ios::binary | std::ios::beg);
		if (!reader.is_open())
		{
			std::cerr << "Could not open GS file!" << std::endl;
			return;
		}

		reader.seekg(0, std::ios::end);
		shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
		reader.seekg(0, std::ios::beg);

		shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());

		if (FAILED(device->CreateGeometryShader(shaderData.c_str(), shaderData.length(), nullptr, &geometryShader)))
		{
			std::cerr << "FAILED TO CREATE GEOMETRY SHADER!" << std::endl;
			return;

		}
		shaderByteCode = shaderData;

		shaderData.clear();
		reader.close();

	}
	void ShutDown()
	{		
		vertexShader->Release();
		hullShader->Release();
		domainShader->Release();
		pixelShader->Release();
		geometryShader->Release();
		matricesBuffer->Release();
		lightBuffer->Release();
	}

	void Render(ID3D11DeviceContext* context, Model* model)
	{
		// Terrain uses the same input layout as models since it is a model 
		context->IASetInputLayout(ShaderData::model_layout);
		// Terrain uses its own shaders
		context->VSSetShader(vertexShader, NULL, 0);
		context->PSSetShader(pixelShader, NULL, 0); // Used for texturing and blending
		context->HSSetShader(hullShader, NULL, 0);
		context->DSSetShader(domainShader, NULL, 0);
		context->GSSetShader(geometryShader, NULL, 0); // Used to recalculate the normals of the triangles
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST); // Used because the tesselation pipeline uses patches

		// Get the matrices (models worldmatrix and the viewPerspecive) and store send them to the domainshader
		XMStoreFloat4x4(&matrices.worldSpace, XMMatrixTranspose(model->GetWorldMatrix()));
		XMMATRIX viewPerspective = XMMatrixTranspose(ShaderData::viewMatrix *ShaderData::perspectiveMatrix);
		XMStoreFloat4x4(&matrices.viewPerspective, viewPerspective);
		UpdateBuffer(context, matricesBuffer, matrices);
		context->DSSetConstantBuffers(0, 1, &matricesBuffer);

		D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};

		// Get, update and set the MTL data for each model
		HRESULT hr = context->Map(*model->GetMTLBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		if FAILED(hr)
		{
			std::cout << "FAILED TO UPDATE MTL BUFFER" << std::endl;
			return;
		}
		memcpy(mappedBuffer.pData, &model->GetMaterial(), sizeof(model->GetMaterial()));
		context->Unmap(*model->GetMTLBuffer(), 0);

		context->PSSetConstantBuffers(1, 1, model->GetMTLBuffer());

		// Update the light for the model used for the shadows (clipspace)
		UpdateBuffer(context, lightBuffer, ShaderData::lightMatrix);
		context->PSSetConstantBuffers(0, 1, &lightBuffer);

		// Textures used
		model->BindTextures(context);								// Bind all the available textures to the pixelshader
		model->BindDisplacementTexture(context, 2, shaders::PS);	// Bind the displacement texture to the pixelshader
		model->BindDisplacementTexture(context);					// Bind the displacement texture to the domainshader
	

		// Set the Vertexbuffer, draw and reset the shaders for the next renderer (particles)
		context->IASetVertexBuffers(0, 1, model->GetBuffer(), &stride, &offset);
		context->Draw(model->GetVertexCount(), 0);

		// Reset
		context->HSSetShader(NULL, NULL, 0);
		context->DSSetShader(NULL, NULL, 0);
		context->GSSetShader(NULL, NULL, 0);
	}
};
