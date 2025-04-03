#pragma once

//C++
#include <memory>
#include <unordered_map>

//Program
#include "SimpleShader.h"
#include "Graphics.h"
#include "PathHelpers.h"
#include "Transform.h"
#include "Camera.h"

//DirectX
#include <DirectXMath.h>

//Toolkit
#include "WICTextureLoader.h"

class Material
{
public:
	Material(const char* name,
		DirectX::XMFLOAT4 colorTint,
		std::shared_ptr<SimpleVertexShader> vs,
		std::shared_ptr<SimplePixelShader> ps,
		float roughness = 0.5f,
		DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1, 1), DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0, 0));
	~Material();

	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState);

	void PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera);

	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT2 GetUvScale();
	DirectX::XMFLOAT2 GetUvOffset();
	float GetRoughness();
	const char* GetName();

	void SetColorTint(DirectX::XMFLOAT4 cT);
	void SetColorTint3(DirectX::XMFLOAT3 cT);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vS);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pS);
	void SetUvScale(DirectX::XMFLOAT2 uvs);
	void SetUvOffset(DirectX::XMFLOAT2 uvo);
	void SetRoughness(float rgh);
	
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& GetTextureSRVMap();

private:
	const char* name;

	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	float roughness;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};

