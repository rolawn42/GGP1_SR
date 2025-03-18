#pragma once

#include<DirectXMath.h>

class Transform
{
public:
	Transform();
	Transform(float x, float y, float z);
	~Transform();

	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 pos);
	void SetRotation(float p, float y, float r);
	void SetRotation(DirectX::XMFLOAT3 rot);
	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 scale);
	
	// Transformers
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 vector);
	void MoveRelative(float x, float y, float z);
	void MoveRelative(DirectX::XMFLOAT3 vector);
	void Rotate(float p, float y, float z);
	void Scale(float x, float y, float r);

	//Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();

	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();
	DirectX::XMFLOAT3 GetForward();
	

	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

private:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 pitchYawRoll; //rotation
	//DirectX::XMFLOAT4 quaternion;
	DirectX::XMFLOAT3 scale;

	DirectX::XMFLOAT3 up;
	DirectX::XMFLOAT3 right;
	DirectX::XMFLOAT3 forward;

	//matrix
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldITMatrix; //world inverse transpose matrix

	bool matriciesDirty;
	bool vectorsDirty;

};

