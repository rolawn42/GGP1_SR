#pragma once

//C++
#include <memory>

//DirectX
#include "DirectXMath.h"

//Program
#include "Transform.h"
#include "Input.h"

class Camera
{
public:
	Camera(DirectX::XMFLOAT3 pos, float moveSpeed, float lookSpeed, float fov, float aspectRatio);
	~Camera();

	//Update Methods
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	//Getters
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	std::shared_ptr<Transform> GetTransform();

private:
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	std::shared_ptr<Transform> transform;

	float moveSpeed;
	float mouseLookSpeed;
	float fov;
	float aspectRatio;
	float nearClipDistance;
	float farClipDistance;
	bool perspective;




};

