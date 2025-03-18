#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat, std::shared_ptr<Transform> transform) :
	transform(transform),
	mesh(mesh),
	material(mat)
{   }

Entity::~Entity()
{
}

void Entity::Draw(std::shared_ptr<Camera> camera)
{
	//this must be done for each entity!

	material->PrepareMaterial(transform, camera);
	mesh->Draw();

	/*
	//collect the data locally
	VertexShaderData dataToCopy{};
	dataToCopy.colorTint = material->GetColorTint();
	dataToCopy.transform = transform->GetWorldMatrix();
	dataToCopy.view = provided->GetView();
	dataToCopy.proj = provided->GetProjection();
	*/

	/*
	D3D11_MAPPED_SUBRESOURCE mapped{};
	Graphics::Context->Map(
		constantBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mapped
	);

	memcpy(mapped.pData, &dataToCopy, sizeof(VertexShaderData));

	Graphics::Context->Unmap(constantBuffer.Get(), 0);
	*/

	/*
	Graphics::Context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we setting right now?
		constantBuffer.GetAddressOf()); // Array of buffers (or address of just one)
	*/
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
	return mesh;
}

std::shared_ptr<Material> Entity::GetMaterial()
{
	return material;
}

std::shared_ptr<Transform> Entity::GetTransform()
{
	return transform;
}

void Entity::SetMaterial(std::shared_ptr<Material> mat)
{
	material = mat;
}
