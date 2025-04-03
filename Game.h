#pragma once

//C++
#include <memory>
#include <vector>
#include "deque"

//Program
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Mesh.h"
#include "Transform.h"
#include "Entity.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Lights.h"
#include "Sky.h"
#include "UI.h"

//DirectX
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

//Toolkit
#include "WICTextureLoader.h"

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
	void CreateLighting();
	void CreateGeometry();

	//Directional Light
	void CreateDirectional(float intensity, DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 direction);

	//Point Light
	void CreatePoint(float intensity, DirectX::XMFLOAT3 color, float range, DirectX::XMFLOAT3 position);

	//SpotLight
	void CreateSpot(float intensity, DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 direction, float range, DirectX::XMFLOAT3 position, float spotInnerAngle, float spotOuterAngle);

	//ALL Lights
	void CreateLight(int type, float intensity, DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 direction, float range, DirectX::XMFLOAT3 position, float spotInnerAngle, float spotOuterAngle);
	
	//Camera
	std::shared_ptr<Camera> currentCamera;
	std::vector<std::shared_ptr<Camera>> cameras;

	//Meshes
	std::vector<std::shared_ptr<Mesh>> meshes;

	//Entities
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
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;


	std::vector<std::shared_ptr<Material>> materials;
	std::vector<std::shared_ptr<SimpleVertexShader>> vss;
	std::vector<std::shared_ptr<SimplePixelShader>> pss;

	//lighting
	DirectX::XMFLOAT3 ambientColor;
	std::vector<Light> lights;

	std::shared_ptr<Sky> sky;

	//imgui
	float backgroundColor[4] = { 0.05f, 0.05f, 0.05f, 0.0f };
};

