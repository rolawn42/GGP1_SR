#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Vertex.h"

class Mesh
{
public:
	Mesh(Vertex vertices[], unsigned int indices[], unsigned int vertexCount, unsigned int indexCount);
	~Mesh();


	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetIndexCount();
	unsigned int GetVertexCount();
	void Draw();

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> comptr_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> comptr_indexBuffer;
	unsigned int i_indexCount;
	unsigned int i_vertexCount;
};

