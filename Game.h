#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"

#include "Mesh.h"
#include "Transform.h"
#include "Entity.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

	//Getters
	DirectX::XMFLOAT4 GetColorTint();
	Transform GetTransform();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void CreateGeometry();

	//Update helper methods
	void UIUpdate(float deltaTime);
	void UIInfo(float deltaTime);
	
	//Camera
	std::shared_ptr<Camera> currentCamera;
	std::vector<std::shared_ptr<Camera>> cameras;

	//Meshes
	std::vector<std::shared_ptr<Mesh>> meshes;

	//Entites
	std::vector<std::shared_ptr<Entity>> entities;

	//Vertex Shader Data
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4 offset;

	Transform transform;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	std::vector<std::shared_ptr<Material>> materials;
	std::vector<std::shared_ptr<SimpleVertexShader>> vss;
	std::vector<std::shared_ptr<SimplePixelShader>> pss;
};

