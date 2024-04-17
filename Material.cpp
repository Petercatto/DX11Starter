#include "Material.h"

//constructor takes in color pixel and vertex shaders
Material::Material(DirectX::XMFLOAT4 color,
	std::shared_ptr<SimplePixelShader> pSPtr, 
	std::shared_ptr<SimpleVertexShader> vSPtr)
{
	colorTint = color;
	pixelShader = pSPtr;
	vertexShader = vSPtr;
}

//returns the color
DirectX::XMFLOAT4 Material::GetColor()
{
	return colorTint;
}

//returns the pixel shader
std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

//returns the vertex shader
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

//sets the color
void Material::SetColor(DirectX::XMFLOAT4 color)
{
	colorTint = color;
}

//sets the pixel shader
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pSPtr)
{
	pixelShader = pSPtr;
}

//sets the vertex shader
void Material::setVertexShader(std::shared_ptr<SimpleVertexShader> vSPtr)
{
	vertexShader = vSPtr;
}

void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ name,srv });
}

void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ name,sampler });
}

void Material::PrepareMaterial()
{
	for (auto& t : textureSRVs) 
	{ 
		pixelShader->SetShaderResourceView(t.first.c_str(), t.second); 
	}
	for (auto& s : samplers) 
	{ 
		pixelShader->SetSamplerState(s.first.c_str(), s.second); 
	}
}
