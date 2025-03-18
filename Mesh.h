#pragma once

#include <d3d11.h>
#include <wrl/client.h>

//C++
#include <vector>
#include <fstream>
#include <stdexcept>

//Program
#include "Vertex.h"
#include "Graphics.h"

//DirectX
#include <DirectXMath.h>

class Mesh
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<UINT> indices);
	Mesh(const char* objFile);
	void CreateBuffers();
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetIndexCount();
	unsigned int GetVertexCount();
	void Draw();

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> comptr_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> comptr_indexBuffer;

	std::vector<DirectX::XMFLOAT3> positions;	// Positions from the file
	std::vector<DirectX::XMFLOAT3> normals;		// Normals from the file
	std::vector<DirectX::XMFLOAT2> uvs;		// UVs from the file
	std::vector<Vertex> verts;		// Verts we're assembling
	std::vector<UINT> indices;
};

