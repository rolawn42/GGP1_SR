#pragma once

//C++
#include <memory>

//DirectX
#include <DirectXMath.h>

//Program
#include "Transform.h"
#include "Mesh.h"
#include "Graphics.h"

struct VertexShaderData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4X4 transform;
};

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> provided);
	~Entity();
	void Draw();

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
private:
	void CreateConstantBuffer();

	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;

	DirectX::XMFLOAT4 colorTint;

	//Constant Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
};

