#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>
#include "Mesh.h"
#include "BufferStructs.h"

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
	void BuildUI(float color[4], DirectX::XMFLOAT4& tint, DirectX::XMFLOAT3& offset);

	//make bgColor a global variable so it can be accessed by the UI
	float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
	//int selectedColor = 0;
	//int previousColor = 0;
	//const char* colors[3] = { "Red", "Green", "Blue" };
	//for showing and hiding the ImGui demo window
	bool ImGuiDemoVisable = false;
	//for UI name
	char textInput[256] = "";
	char nameUI[256] = "Peter";

	//vertext shader data variables
	VertexShaderData vsData;
	DirectX::XMFLOAT4 _colorTint = DirectX::XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	DirectX::XMFLOAT3 _offset = DirectX::XMFLOAT3(0.25f, 0.0f, 0.0f);

	//meshes
	std::shared_ptr<Mesh> triangle;
	std::shared_ptr<Mesh> square;
	std::shared_ptr<Mesh> star;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	//constant buffer for vertex shader data
	Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer;
	
	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

};

