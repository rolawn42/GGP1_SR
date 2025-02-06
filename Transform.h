#pragma once

#include<DirectXMath.h>

class Transform
{
public:
	Transform();
	~Transform();

	// Setters
	void SetPosition(float x, float y, float z);
	//void SetPosition(DirectX::XMFLOAT3 pos);
	void SetRotation(float p, float y, float r);
	void SetScale(float x, float y, float z);
	
	// Transformers
	void MoveAbsolute(float x, float y, float z);
	//void MoveRelative(float x, float y, float z);
	void Rotate(float p, float y, float z);
	void Scale(float x, float y, float z);

	//Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();

	DirectX::XMFLOAT4X4 GetWorldMatrix();
	//DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

private:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 pitchYawRoll; //rotation
	//DirectX::XMFLOAT4 quaternion;
	DirectX::XMFLOAT3 scale;


	//matrix
	DirectX::XMFLOAT4X4 worldMatrix;

	bool dirty;

};

