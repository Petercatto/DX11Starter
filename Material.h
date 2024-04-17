#pragma once
#include "SimpleShader.h"
#include <DirectXMath.h>
#include <memory>
#include <unordered_map>

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
	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	void PrepareMaterial();

private:
	//variables for materials
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	//textures
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};

