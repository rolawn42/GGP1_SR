#pragma once

//Program
#include "SimpleShader.h"
#include "Graphics.h"
#include "Camera.h"
#include "Mesh.h"

//DirectX
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>

//Toolkit
#include "WICTextureLoader.h"

class Sky
{
public:
	Sky(Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState, std::shared_ptr<Mesh> mesh, std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader);

	void CreateCubemap(const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

	//Draw
	void Draw(std::shared_ptr<Camera> camera);


private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthBuffer;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer;

	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;

};

