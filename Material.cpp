#include "Material.h"

using namespace DirectX;

Material::Material(const char* name, DirectX::XMFLOAT4 colorTint, std::shared_ptr<SimpleVertexShader> vs, std::shared_ptr<SimplePixelShader> ps, float roughness, DirectX::XMFLOAT2 uvScale, DirectX::XMFLOAT2 uvOffset) :
	name(name),
	colorTint(colorTint),
	vertexShader(vs),
	pixelShader(ps), 
	uvScale(uvScale),
	uvOffset(uvOffset),
	roughness(roughness)
{
}

Material::~Material()
{
}

void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ name, srv });
}

void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ name, sampler });
}

void Material::PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera)
{
	// Turn on these shaders
	vertexShader->SetShader();
	pixelShader->SetShader();

	// Send data to the vertex shader
	vertexShader->SetMatrix4x4("world", transform->GetWorldMatrix());
	vertexShader->SetMatrix4x4("worldInvTranspose", transform->GetWorldInverseTransposeMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("proj", camera->GetProjection());
	vertexShader->CopyAllBufferData();

	// Send data to the pixel shader
	pixelShader->SetFloat4("colorTint", colorTint);
	pixelShader->SetFloat2("uvScale", uvScale);
	pixelShader->SetFloat2("uvOffset", uvOffset);
	pixelShader->SetFloat("roughness", roughness);

	pixelShader->SetFloat3("cameraPosition", camera->GetTransform()->GetPosition());
	
	for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplers) { pixelShader->SetSamplerState(s.first.c_str(), s.second); }

	pixelShader->CopyAllBufferData();

}

DirectX::XMFLOAT4 Material::GetColorTint()
{
	return colorTint;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

DirectX::XMFLOAT2 Material::GetUvScale()
{
	return uvScale;
}

DirectX::XMFLOAT2 Material::GetUvOffset()
{
	return uvOffset;
}

float Material::GetRoughness()
{
	return roughness;
}

const char* Material::GetName()
{
	return name;
}

void Material::SetColorTint(DirectX::XMFLOAT4 cT)
{
	colorTint = cT;
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vS)
{
	vertexShader = vS;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pS)
{
	pixelShader = pS;
}

void Material::SetUvScale(DirectX::XMFLOAT2 uvs)
{
	uvScale = uvs;
}

void Material::SetUvOffset(DirectX::XMFLOAT2 uvo)
{
	uvOffset = uvo;
}

void Material::SetRoughness(float rgh)
{
	roughness = rgh;
}

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& Material::GetTextureSRVMap()
{
	return textureSRVs;
}


