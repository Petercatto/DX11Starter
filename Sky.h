#pragma once

#include "DXCore.h"
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
#include "SimpleShader.h"
#include "WICTextureLoader.h"
#include "Camera.h"

class Sky
{
private:
	//sky variables
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeMapSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthBuffer;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	std::shared_ptr<Mesh> cube;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
public:
	//constructor
	Sky(std::shared_ptr<Mesh> meshPtr,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> s,
		Microsoft::WRL::ComPtr<ID3D11Device> d,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> c,
		std::shared_ptr<SimplePixelShader> pSPtr,
		std::shared_ptr<SimpleVertexShader> vSPtr,
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

	// --------------------------------------------------------
	// Author: Chris Cascioli
	// --------------------------------------------------------
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right, 
		const wchar_t* left, 
		const wchar_t* up, 
		const wchar_t* down, 
		const wchar_t* front, 
		const wchar_t* back);

	//draw method
	void Draw(std::shared_ptr<Camera> camera);
};

