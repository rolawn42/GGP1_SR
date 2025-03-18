#include "Material.h"

using namespace DirectX;

Material::Material(DirectX::XMFLOAT4 colorTint, std::shared_ptr<SimpleVertexShader> vs, std::shared_ptr<SimplePixelShader> ps) :
	colorTint(colorTint), 
	vertexShader(vs),
	pixelShader(ps)
{  }

Material::~Material()
{
}

void Material::PrepareMaterial(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera)
{
	// Turn on these shaders
	vertexShader->SetShader();
	pixelShader->SetShader();

	// Send data to the vertex shader
	vertexShader->SetMatrix4x4("world", transform->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("proj", camera->GetProjection()); //CHANGED: this was named wrong, it was 'projection before, my VS had is as 'proj'. this is why nothing was showing
	vertexShader->CopyAllBufferData();

	// Send data to the pixel shader
	pixelShader->SetFloat4("colorTint", colorTint);
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
