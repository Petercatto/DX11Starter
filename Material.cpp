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
