#pragma once

//C++
#include <memory>

//Program
#include "SimpleShader.h"
#include "Graphics.h"
#include "PathHelpers.h"
#include "Transform.h"
#include "Camera.h"

//DirectX
#include <DirectXMath.h>

class Material
{
public:
	Material(DirectX::XMFLOAT4 colorTint, std::shared_ptr<SimpleVertexShader> vs, std::shared_ptr<SimplePixelShader> ps);
	~Material();

	void PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera);

	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	void SetColorTint(DirectX::XMFLOAT4 cT);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vS);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pS);
private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
};

