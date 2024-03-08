#include "Material.h"

//constructor takes in color pixel and vertex shaders
Material::Material(DirectX::XMFLOAT4 color,
	float rough,
	std::shared_ptr<SimplePixelShader> pSPtr, 
	std::shared_ptr<SimpleVertexShader> vSPtr)
{
	colorTint = color;
	pixelShader = pSPtr;
	vertexShader = vSPtr;
	roughness = rough;
}

//returns the color
DirectX::XMFLOAT4 Material::GetColor()
{
	return colorTint;
}

//returns the roughness
float Material::GetRoughness()
{
	return roughness;
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

//sets the roughness
void Material::SetRoughness(float rough)
{
	roughness = rough;
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
