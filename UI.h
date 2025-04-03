#pragma once

//C++
#include <memory>
#include <vector>
#include <functional>
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

//ImGui
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

void UIInfo(float deltatime);
void UIUpdate(float deltatime,
	std::shared_ptr<Camera> currentCamera, 
	std::vector<std::shared_ptr<Camera>> cameras,
	std::vector<std::shared_ptr<Mesh>> meshes,
	std::vector<std::shared_ptr<Entity>> entities,
	std::vector<std::shared_ptr<Material>> materials, 
	std::vector<Light> lights);

void DF1(const char* name, float startValue, std::function<void(float)> endLocation);
void DF2(const char* name, DirectX::XMFLOAT2 startValue, std::function<void(DirectX::XMFLOAT2)> endLocation);
void DF3(const char* name, DirectX::XMFLOAT3 startValue, std::function<void(DirectX::XMFLOAT3)> endLocation);

const char* LightType(int num);



