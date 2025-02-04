#include "Vertex.h"
#include "Graphics.h"

#include "Mesh.h"

#include <DirectXMath.h>

// For the DirectX Math library
using namespace DirectX;

Mesh::Mesh(Vertex vertices[], unsigned int indices[], unsigned int vertexCount, unsigned int indexCount)
{
	i_vertexCount = vertexCount;
	i_indexCount = indexCount;

	//create vertex buffer
	//define buffer desc struct
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	vbd.ByteWidth = sizeof(Vertex) * i_vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells Direct3D this is a vertex buffer
	vbd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	//create the struct with data
	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = vertices; // pSysMem = Pointer to System Memory

	//create the buffer on the gpu with parameters
	Graphics::Device->CreateBuffer(&vbd, &initialVertexData, comptr_vertexBuffer.GetAddressOf());

	//create the index buffer
	//define buffer desc struct
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	ibd.ByteWidth = sizeof(unsigned int) * i_indexCount;	// 3 = number of indices in the buffer
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
	ibd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	//create struct with data
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indices; // pSysMem = Pointer to System Memory

	//send to the gpu
	Graphics::Device->CreateBuffer(&ibd, &initialIndexData, comptr_indexBuffer.GetAddressOf());
}

Mesh::~Mesh() { }

unsigned int Mesh::GetIndexCount() {
	return i_indexCount;
}

unsigned int Mesh::GetVertexCount() {
	return i_vertexCount;
}

void Mesh::Draw() {
	//create buffers for primitve / input assembly
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::Context->IASetVertexBuffers(0, 1, comptr_vertexBuffer.GetAddressOf(), &stride, &offset);
	Graphics::Context->IASetIndexBuffer(comptr_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//tell direct3d what to draw
	Graphics::Context->DrawIndexed(
		i_indexCount,     // The number of indices to use (we could draw a subset if we wanted)
		0,     // Offset to the first index we want to use
		0);    // Offset to add to each index when looking up vertices
}