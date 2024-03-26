#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include "Mesh.h"
#include "GameEntity.h"
#include <vector>
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Lights.h"
#include "WICTextureLoader.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders(); 
	void CreateGeometry();
	void ImGuiUpdate(float deltaTime);
	void BuildUI(float color[4], DirectX::XMFLOAT4X4& world);

	//make bgColor a global variable so it can be accessed by the UI
	float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
	//for showing and hiding the ImGui demo window
	bool ImGuiDemoVisable = false;
	//for UI name
	char textInput[256] = "";
	char nameUI[256] = "Peter";
	//for cameras
	int selectedCamera = 0;

	//vertex shader data variables
	DirectX::XMFLOAT4X4 _world;

	//meshes
	std::shared_ptr<Mesh> triangle;
	std::shared_ptr<Mesh> square;
	std::shared_ptr<Mesh> star;
	std::shared_ptr<Mesh> cube;
	std::shared_ptr<Mesh> cylinder;
	std::shared_ptr<Mesh> helix;
	std::shared_ptr<Mesh> quad;
	std::shared_ptr<Mesh> doubleSidedQuad;
	std::shared_ptr<Mesh> torus;
	std::shared_ptr<Mesh> sphere;

	//textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularSRV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

	//materials
	std::vector<std::shared_ptr<Material>> materials;

	//entity vector
	std::vector<std::shared_ptr<GameEntity>> entities;

	//cameras
	std::vector<std::shared_ptr<Camera>> cameras;
	std::shared_ptr<Camera> activeCamera;

	//lighting
	DirectX::XMFLOAT3 ambientColor = { 0.1f, 0.15f, 0.1817f };
	std::vector<Light> lights;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	
	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;

	std::shared_ptr<SimplePixelShader> customShader;
};

