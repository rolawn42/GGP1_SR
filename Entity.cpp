#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> provided) : 
	transform(std::make_shared<Transform>())
{
	mesh = provided;

	colorTint = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	CreateConstantBuffer();
}

Entity::~Entity()
{
}

void Entity::Draw()
{
	//1 bind the constant buffer resource for the vertex shader stage
	//2 collect the *current* entitiess data in a c++ struct (needs to now hold the world matrix of your entity)
	//3 map / memcpy / unmap the constant buffer resource
	//4 set the correct vertex and index buffers
	//5 tell direct 3d to render using the currently bould resources

	//this must be done for each entity!

	mesh->Draw();

	//collect the data locally
	VertexShaderData dataToCopy{};
	dataToCopy.colorTint = colorTint;
	dataToCopy.transform = transform->GetWorldMatrix();

	D3D11_MAPPED_SUBRESOURCE mapped{};
	Graphics::Context->Map(
		constantBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mapped
	);

	unsigned int size = sizeof(VertexShaderData);
	size = ((size + 15) / 16) * 16;

	memcpy(mapped.pData, &dataToCopy, size);

	Graphics::Context->Unmap(constantBuffer.Get(), 0);

	Graphics::Context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we setting right now?
		constantBuffer.GetAddressOf()); // Array of buffers (or address of just one)
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
	return mesh;
}

std::shared_ptr<Transform> Entity::GetTransform()
{
	return transform;
}

void Entity::CreateConstantBuffer()
{
	//create constant buffer
	D3D11_BUFFER_DESC cbDesc{};
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = 32;
	cbDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	Graphics::Device->CreateBuffer(&cbDesc, 0, constantBuffer.GetAddressOf());

	Graphics::Context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
}
