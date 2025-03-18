#pragma once

//C++
#include <memory>

//DirectX
#include <DirectXMath.h>

//Program
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Graphics.h"
#include "Camera.h"

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> mat, std::shared_ptr<Transform> transform);
	~Entity();
	void Draw(std::shared_ptr<Camera> provided);

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	std::shared_ptr<Transform> GetTransform();

	void SetMaterial(std::shared_ptr<Material> mat);
private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};

