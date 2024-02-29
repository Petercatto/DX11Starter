#pragma once
#include "SimpleShader.h"
#include <DirectXMath.h>
#include <memory>

class Material
{
public:
	//constructor
	Material(DirectX::XMFLOAT4 color, 
		std::shared_ptr<SimplePixelShader> pSPtr, 
		std::shared_ptr<SimpleVertexShader> vSPtr);

	//getters
	DirectX::XMFLOAT4 GetColor();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();

	//setters
	void SetColor(DirectX::XMFLOAT4 color);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pSPtr);
	void setVertexShader(std::shared_ptr<SimpleVertexShader> vSPtr);

private:
	//variables for materials
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
};

