#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include <DirectXMath.h>

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
	void LoadShaders();
	void CreateGeometry();

	//Update helper methods
	void UIUpdate(float deltaTime);
	void UIInfo(float deltaTime);

	//Meshes
	std::vector<std::shared_ptr<Mesh>> apmesh_meshes;
	unsigned int i_meshCount;

	//Entites
	std::vector<std::shared_ptr<Entity>> apentity_entities;
	unsigned int i_entityCount;

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

	//Constant Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
};

