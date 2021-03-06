#include "Mesh.h"
#include <fstream>

// OBJ reader
bool Mesh::LoadModel(std::string name)
{
	std::vector<std::array<float, 3>> v;
	std::vector<std::array<float, 2>> vt;
	std::vector<std::array<float, 3>> vn;
	std::vector<std::array<std::array<int, 3>, 3>> f;

	std::ifstream reader;

	reader.open("Models/" + name + "/" + name + ".obj", std::ios::beg); // Open the .obj-file

	if (!reader.is_open())
		return false;

	std::string line;
	while (std::getline(reader, line))
	{
		std::string text;
		reader >> text;
		if (text == "v") // v is a vertex
		{
			std::array<float, 3> vertex;

			reader >> vertex[0];	// load the first float to index 0
			reader >> vertex[1];	// load the second float to index 1
			reader >> vertex[2];	// load the thrid float to index 2

			v.push_back(vertex);
		}

		if (text == "vt") // vt is the texture coordinate of one vertex
		{
			std::array<float, 2> texCoord;

			reader >> texCoord[0]; 	  // load the first float to index 0
			reader >> texCoord[1];	  // load the second float to index 1
									  
			texCoord[1] = 1 - texCoord[1];	// inverting V because directX reads it upsidedown otherwise

			vt.push_back(texCoord);
		}

		if (text == "vn") // vn is the normal of one vertex
		{
			std::array<float, 3> normal;

			reader >> normal[0];
			reader >> normal[1];
			reader >> normal[2];

			vn.push_back(normal);
		}

		if (text == "f") // f is a face containing 3 vertices
		{
			std::array<std::array<int, 3>, 3> face;

			std::string temp;

			for (int i = 0; i < 3; ++i)
			{
				reader >> temp;

				std::string substr;
			
				// in an obj:
				// f 1/2/3 4/5/6 7/8/9
				//    v0	v1	  v2     position, texcoord(uv), normal

				// stoi = string to int

				face[0][i] = std::stoi(temp.substr(0, temp.find("/")));								 // position 

				substr = temp.substr(temp.find("/") + 1, temp.length());							 // new substring -> from where we found the last index to the end of the line
				face[1][i] = std::stoi(substr.substr(0, substr.find("/")));							 // texcoord
				face[2][i] = std::stoi(substr.substr(substr.find("/") + 1, substr.length()));		 // normal
			}

			f.push_back(face);
		}
	}

	for (int i = 0; i < f.size(); ++i)
	{
		std::array<float, 3> pos;
		std::array<float, 2> tex;
		std::array<float, 3> nor;
		std::array<Vertex, 3> vertices;

		for (int j = 0; j < 3; ++j)
		{
			// Position of vJ
			int index = f[i][0][j] - 1;

			pos[0] = v[index][0];
			pos[1] = v[index][1];
			pos[2] = v[index][2];

			// TexCoord of vJ
			index = f[i][1][j] - 1;

			tex[0] = vt[index][0];
			tex[1] = vt[index][1];

			// Normal of vJ
			index = f[i][2][j] - 1;

			nor[0] = vn[index][0];
			nor[1] = vn[index][1];
			nor[2] = vn[index][2];

			positions.push_back({ pos[0],pos[1],pos[2] });		//For shadow
			vertices[j] = Vertex(pos, tex, nor);
			vertexCount++;
		}

		Face face = Face(vertices[0], vertices[1], vertices[2]);
		faces.push_back(face); // data for the vertexbuffer
	}

	return true;
}

// Material Reader
bool Mesh::LoadMaterial(ID3D11Device* device, std::string name)
{
	std::ifstream reader;

	reader.open("Models/" + name + "/" + name + ".mtl", std::ios::beg);		// Opens the MTL-file

	if (!reader.is_open())
		return false;

	int counter = 0;
	std::string line;
	while (std::getline(reader, line))
	{
		std::string text;
		reader >> text;

		if (text == "Kd")										// Find the diffuse and store it in the struct
		{
			XMFLOAT4 diffuse;

			reader >> diffuse.x;
			reader >> diffuse.y;
			reader >> diffuse.z;
			diffuse.w = 0;

			material.data.diffuse = diffuse;
		}

		if (text == "Ka")										// Find the ambient and store it in the struct
		{
			XMFLOAT4 ambient;

			reader >> ambient.x;
			reader >> ambient.y;
			reader >> ambient.z;
			ambient.w = 0;

			material.data.ambient = ambient;
		}

		if (text == "Ks")										// Find the specular and store it in the struct
		{
			XMFLOAT4 specular;

			reader >> specular.x;
			reader >> specular.y;
			reader >> specular.z;
			specular.w = 0;

			material.data.specular = specular;
		}

		if (text == "map_Kd")									// Find the texture used, create it and store it in the material-vector
		{
			std::string diffuse;
			reader >> diffuse;
			material.diffuseTexures.emplace_back(new Texture(device, "Models/" + name + "/" + diffuse));	//place into the container
		}

	}

	// Create the buffer contaning the material-data, per model
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(material.data);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // A resource that is accessible by both the GPU(read only) and the CPU(write only).A dynamic resource is a good choice for a resource that will be updated by the CPU at least once per frame
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // The resource is to be mappable so that the CPU can change its contents
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &mtlBuffer);
	if FAILED(hr)
	{
		std::cout << "FAILED TO CREATE BUFFER" << std::endl;
	}
	return true;
}

Mesh::~Mesh()
{
	
}
