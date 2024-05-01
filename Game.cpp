#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"

//ImGui includes
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"Game",			// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	//load all assets and create entities
	LoadAssetsAndCreateEntities();
	CreateAndLoadLights();

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();

	CreateShadowMapResources();

	//create the particle resources
	CreateParticleResources();

	//sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());
	CreatePostProcessResources();
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"PixelShader.cso").c_str());

	customShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"CustomPS.cso").c_str());

	skyVertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"SkyVertexShader.cso").c_str());
	skyPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"SkyPixelShader.cso").c_str());

	shadowVertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"ShadowVertexShader.cso").c_str());

	ppVertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"ppVertexShader.cso").c_str());
	blurPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"blurPixelShader.cso").c_str());
	chromaticPixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"chromaticPixelShader.cso").c_str());

	particleVertexShader = std::make_shared<SimpleVertexShader>(device, context,
		FixPath(L"particleVertexShader.cso").c_str());
	particlePixelShader = std::make_shared<SimplePixelShader>(device, context,
		FixPath(L"particlePixelShader.cso").c_str());
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex triVerts[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.5f, -0.5f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(-0.5f, -0.5f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	UINT triIndices[] = { 0, 1, 2 };

	int numTriVerts = sizeof(triVerts) / sizeof(triVerts[0]);
	int numTriIndices = sizeof(triIndices) / sizeof(triIndices[0]);
	//create triangle mesh
	triangle = std::make_shared<Mesh>(context, device, triVerts, numTriVerts, triIndices, numTriIndices);

	//square
	Vertex squareVerts[] =
	{
		{ XMFLOAT3(-0.8f, +0.8f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(-0.5f, +0.8f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(-0.8f, +0.5f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(-0.5f, +0.5f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
	};

	UINT squareIndices[] = { 0, 1, 2, 2, 1, 3 };

	int numSquareVerts = sizeof(squareVerts) / sizeof(squareVerts[0]);
	int numSquareIndices = sizeof(squareIndices) / sizeof(squareIndices[0]);

	square = std::make_shared<Mesh>(context, device, squareVerts, numSquareVerts, squareIndices, numSquareIndices);

	//star
	Vertex starVerts[] =
	{
		{ XMFLOAT3(+0.7f, +0.6f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.7f, +0.45f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.55f, +0.3f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.6f, +0.55f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.5f, +0.7f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.65f, +0.7f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.7f, +0.95f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.75f, +0.7f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.9f, +0.7f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.8f, +0.55f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
		{ XMFLOAT3(+0.85f, +0.3f, +5.0f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0)},
	};

	UINT starIndices[] = { 1, 2, 0, 0, 2, 3, 3, 4, 0, 0, 4, 5, 5, 6, 0, 0, 6, 7, 7, 8, 0, 0, 8, 9, 9, 10, 0, 0, 10, 1 };

	int numStarVerts = sizeof(starVerts) / sizeof(starVerts[0]);
	int numStarIndices = sizeof(starIndices) / sizeof(starIndices[0]);

	star = std::make_shared<Mesh>(context, device, starVerts, numStarVerts, starIndices, numStarIndices);

	//3D Models
	cube = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cube.obj").c_str(), context, device);
	cylinder = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/cylinder.obj").c_str(), context, device);
	helix = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/helix.obj").c_str(), context, device);
	quad = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad.obj").c_str(), context, device);
	doubleSidedQuad = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str(), context, device);
	torus = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/torus.obj").c_str(), context, device);
	sphere = std::make_shared<Mesh>(FixPath(L"../../Assets/Models/sphere.obj").c_str(), context, device);


	//grid ground snow
	const int gridSize = 64;
	const float gridSpacing = 0.45f;

	std::vector<Vertex> gridVerts;
	std::vector<UINT> gridIndices;

	for (int y = 0; y < gridSize; y++)
	{
		for (int x = 0; x < gridSize; x++)
		{
			float xPos = static_cast<float>(x) * gridSpacing;
			float yPos = static_cast<float>(y) * gridSpacing;

			Vertex v;
			v.Position = XMFLOAT3(xPos, 0.0f, yPos);
			v.Normal = XMFLOAT3(0.0f, 1.0f, 0.0f); //assuming normal points straight up
			v.UV = XMFLOAT2(static_cast<float>(x) / (gridSize - 1), static_cast<float>(y) / (gridSize - 1));

			gridVerts.push_back(v);

			//skip creating triangles for the last row and column
			if (x < gridSize - 1 && y < gridSize - 1)
			{
				//create two triangles for each grid cell
				gridIndices.push_back(y * gridSize + x);               //current
				gridIndices.push_back((y + 1) * gridSize + x);         //next row
				gridIndices.push_back(y * gridSize + x + 1);           //next column

				gridIndices.push_back(y * gridSize + x + 1);           //current
				gridIndices.push_back((y + 1) * gridSize + x);         //next row
				gridIndices.push_back((y + 1) * gridSize + x + 1);     //next row, next column
			}
		}
	}

	int numGridVerts = (int)gridVerts.size();
	int numGridIndices = (int)gridIndices.size();

	// Assuming 'grid' is a std::shared_ptr<Mesh>'
	snowPlane = std::make_shared<Mesh>(context, device, gridVerts.data(), numGridVerts, gridIndices.data(), numGridIndices);
}

//ImGui update helper function
void Game::ImGuiUpdate(float deltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input& input = Input::GetInstance();
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);
}

//Builds UI utilizing ImGui
void Game::BuildUI(DirectX::XMFLOAT4X4& world)
{
	//beginning of window
	ImGui::Begin((std::string(nameUI) + "'s Window").c_str());

	//shows the current frame rate
	ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);

	ImGui::Image(blurSRV.Get(), ImVec2(512, 512));

	//shows the current window size
	ImGui::Text("Window Resolution: %dx%d", windowWidth, windowHeight);
	
	//text input example
	ImGui::InputText("Enter Name", textInput, IM_ARRAYSIZE(textInput));

	//copies the contents of text input into the name UI
	if (ImGui::Button("Change Window Name"))
	{
		strncpy_s(nameUI, textInput, IM_ARRAYSIZE(nameUI));
	}

	//demo window visibility button to show and hide the demo window
	if (ImGui::Button("ImGui Demo Window"))
	{
		ImGuiDemoVisable = !ImGuiDemoVisable;
	}
	if (ImGuiDemoVisable)
	{
		ImGui::ShowDemoWindow();
	}

	ImGui::SameLine();

	//check box example
	ImGui::Checkbox("Show/Hide", &ImGuiDemoVisable);

	//info for the current camera
	if (ImGui::TreeNode("Cameras"))
	{
		//camera changer
		for (int i = 0; i < cameras.size(); i++)
		{
			if (ImGui::RadioButton(("Camera " + std::to_string(i + 1)).c_str(), &selectedCamera, i))
			{
				activeCamera = cameras[selectedCamera];
			}
			if (i != cameras.size() - 1)
			{
				ImGui::SameLine();
			}
		}

		ImGui::Text("Active Camera:");
		ImGui::Text("Position: %.2f, %.2f, %.2f", activeCamera->GetTransform().GetPosition().x, activeCamera->GetTransform().GetPosition().y, activeCamera->GetTransform().GetPosition().z);
		ImGui::Text("Field of View: %.2f degrees", DirectX::XMConvertToDegrees(activeCamera->GetFOV()));
		if (activeCamera->GetType())
		{
			ImGui::Text("Projection: Perspective");
		}
		else
		{
			ImGui::Text("Projection: Orthographic");
		}
		//close the entire list
		ImGui::TreePop();
	}

	//entity list
	if (ImGui::TreeNode("Scene Entities"))
	{
		//for each entity
		for (int i = 0; i < entities.size(); i++)
		{
			//create a new list entry that shows the edits and values as well as the mesh index count
			if (ImGui::TreeNode((std::string("Entity ") + std::to_string(i)).c_str()))
			{
				//get the transforms for the current entity
				auto& transform = entities[i]->GetTransform();

				//display and edit position
				auto position = transform.GetPosition();
				ImGui::DragFloat3("Position", &position.x, 0.1f);
				transform.SetPosition(position);

				//display and edit rotation
				auto rotation = transform.GetPitchYawRoll();
				ImGui::DragFloat3("Rotation (Radians)", &rotation.x, 0.1f);
				transform.SetRotation(rotation);

				//display and edit scale
				auto scale = transform.GetScale();
				ImGui::DragFloat3("Scale", &scale.x, 0.1f);
				transform.SetScale(scale);

				//mesh index count
				ImGui::BulletText("Mesh Index Count: %d", entities[i]->GetMesh()->GetIndexCount());

				//close the current entity
				ImGui::TreePop();
			}
		}
		//close the entire list
		ImGui::TreePop();
	}

	//light list
	if (ImGui::TreeNode("Scene Lights"))
	{
		//for each entity
		for (int i = 0; i < lights.size(); i++)
		{
			//create a new list entry that shows the edits and values as well as the mesh index count
			if (ImGui::TreeNode((std::string("Light ") + std::to_string(i)).c_str()))
			{
				ImGui::RadioButton("Directional", &lights[i].Type, LIGHT_TYPE_DIRECTIONAL);
				ImGui::SameLine();
				ImGui::RadioButton("Point", &lights[i].Type, LIGHT_TYPE_POINT);

				if (lights[i].Type == LIGHT_TYPE_DIRECTIONAL)
				{
					ImGui::DragFloat3("Direction", &lights[i].Direction.x, 0.01f, -1.0f, 1.0f);
				}
				else if (lights[i].Type == LIGHT_TYPE_POINT)
				{
					ImGui::DragFloat3("Position", &lights[i].Position.x, 0.01f);
					ImGui::DragFloat("Range", &lights[i].Range, 0.01f, 0.0f, FLT_MAX);
				}

				ImGui::ColorEdit3("Color", &lights[i].Color.x);
				ImGui::DragFloat("Intensity", &lights[i].Intensity, 0.01f, 0.0f, 1.0f);

				//close the current entity
				ImGui::TreePop();
			}
		}
		//close the entire list
		ImGui::TreePop();
	}

	//post processing
	ImGui::SliderInt("Blur Radius", &blurRadius, 0, 10);
	ImGui::SliderFloat3("Chromatic Aberration", &colorOffset.x, -5.0f, 5.0f);

	//ending of the window
	ImGui::End();
}

void Game::LoadAssetsAndCreateEntities()
{
	//create sampler state
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 10;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, sampler.GetAddressOf());

	//load textures
	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/bronze_albedo.png").c_str(), 0, bronzeAlbedo.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/bronze_metal.png").c_str(), 0, bronzeMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/bronze_normals.png").c_str(), 0, bronzeNormals.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/bronze_roughness.png").c_str(), 0, bronzeRoughness.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_albedo.png").c_str(), 0, cobbleAlbedo.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_metal.png").c_str(), 0, cobbleMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_normals.png").c_str(), 0, cobbleNormals.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_roughness.png").c_str(), 0, cobbleRoughness.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/floor_albedo.png").c_str(), 0, floorAlbedo.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/floor_metal.png").c_str(), 0, floorMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/floor_normals.png").c_str(), 0, floorNormals.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/floor_roughness.png").c_str(), 0, floorRoughness.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/paint_albedo.png").c_str(), 0, paintAlbedo.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/paint_metal.png").c_str(), 0, paintMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/paint_normals.png").c_str(), 0, paintNormals.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/paint_roughness.png").c_str(), 0, paintRoughness.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/rough_albedo.png").c_str(), 0, roughAlbedo.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/rough_metal.png").c_str(), 0, roughMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/rough_normals.png").c_str(), 0, roughNormals.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/rough_roughness.png").c_str(), 0, roughRoughness.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/scratched_albedo.png").c_str(), 0, scratchedAlbedo.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/scratched_metal.png").c_str(), 0, scratchedMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/scratched_normals.png").c_str(), 0, scratchedNormals.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/scratched_roughness.png").c_str(), 0, scratchedRoughness.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/wood_albedo.png").c_str(), 0, woodAlbedo.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/wood_metal.png").c_str(), 0, woodMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/wood_normals.png").c_str(), 0, woodNormals.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/wood_roughness.png").c_str(), 0, woodRoughness.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/snow.png").c_str(), 0, snowSRV.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/snow_albedo.png").c_str(), 0, snowAlbedo.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/snow_metal.png").c_str(), 0, snowMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/snow_normals.png").c_str(), 0, snowNormals.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		FixPath(L"../../Assets/Textures/snow_roughness.png").c_str(), 0, snowRoughness.GetAddressOf());

	//make sky
	sky = std::make_shared<Sky>(cube, sampler, device, context, skyPixelShader, skyVertexShader,
		FixPath(L"../../Assets/Textures/right.png").c_str(),
		FixPath(L"../../Assets/Textures/left.png").c_str(),
		FixPath(L"../../Assets/Textures/up.png").c_str(),
		FixPath(L"../../Assets/Textures/down.png").c_str(),
		FixPath(L"../../Assets/Textures/front.png").c_str(),
		FixPath(L"../../Assets/Textures/back.png").c_str());

	//set sampler state
	skyPixelShader->SetSamplerState("BasicSampler", sampler);

	//make materials
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), pixelShader, vertexShader));	//red
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), pixelShader, vertexShader));	//green
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), pixelShader, vertexShader));	//blue
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), customShader, vertexShader));	//custom pixel shader
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), pixelShader, vertexShader));	//bronze
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), pixelShader, vertexShader));	//cobble
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), pixelShader, vertexShader));	//floor
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), pixelShader, vertexShader));	//paint
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), pixelShader, vertexShader));	//rough
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), pixelShader, vertexShader));  //scratched
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), pixelShader, vertexShader));	//wood
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), pixelShader, vertexShader));	//snow

	//add textures
	materials[4]->AddTextureSRV("Albedo", bronzeAlbedo);
	materials[4]->AddTextureSRV("NormalMap", bronzeNormals);
	materials[4]->AddTextureSRV("RoughnessMap", bronzeRoughness);
	materials[4]->AddTextureSRV("MetalnessMap", bronzeMetal);
	materials[4]->AddSampler("BasicSampler", sampler);

	materials[5]->AddTextureSRV("Albedo", cobbleAlbedo);
	materials[5]->AddTextureSRV("NormalMap", cobbleNormals);
	materials[5]->AddTextureSRV("RoughnessMap", cobbleRoughness);
	materials[5]->AddTextureSRV("MetalnessMap", cobbleMetal);
	materials[5]->AddSampler("BasicSampler", sampler);

	materials[6]->AddTextureSRV("Albedo", floorAlbedo);
	materials[6]->AddTextureSRV("NormalMap", floorNormals);
	materials[6]->AddTextureSRV("RoughnessMap", floorRoughness);
	materials[6]->AddTextureSRV("MetalnessMap", floorMetal);
	materials[6]->AddSampler("BasicSampler", sampler);

	materials[7]->AddTextureSRV("Albedo", paintAlbedo);
	materials[7]->AddTextureSRV("NormalMap", paintNormals);
	materials[7]->AddTextureSRV("RoughnessMap", paintRoughness);
	materials[7]->AddTextureSRV("MetalnessMap", paintMetal);
	materials[7]->AddSampler("BasicSampler", sampler);

	materials[8]->AddTextureSRV("Albedo", roughAlbedo);
	materials[8]->AddTextureSRV("NormalMap", roughNormals);
	materials[8]->AddTextureSRV("RoughnessMap", roughRoughness);
	materials[8]->AddTextureSRV("MetalnessMap", roughMetal);
	materials[8]->AddSampler("BasicSampler", sampler);

	materials[9]->AddTextureSRV("Albedo", scratchedAlbedo);
	materials[9]->AddTextureSRV("NormalMap", scratchedNormals);
	materials[9]->AddTextureSRV("RoughnessMap", scratchedRoughness);
	materials[9]->AddTextureSRV("MetalnessMap", scratchedMetal);
	materials[9]->AddSampler("BasicSampler", sampler);

	materials[10]->AddTextureSRV("Albedo", woodAlbedo);
	materials[10]->AddTextureSRV("NormalMap", woodNormals);
	materials[10]->AddTextureSRV("RoughnessMap", woodRoughness);
	materials[10]->AddTextureSRV("MetalnessMap", woodMetal);
	materials[10]->AddSampler("BasicSampler", sampler);

	materials[11]->AddTextureSRV("Albedo", snowAlbedo);
	materials[11]->AddTextureSRV("NormalMap", snowNormals);
	materials[11]->AddTextureSRV("RoughnessMap", snowRoughness);
	materials[11]->AddTextureSRV("MetalnessMap", snowMetal);
	materials[11]->AddSampler("BasicSampler", sampler);


	//push all the entities
	entities.push_back(std::make_shared<GameEntity>(triangle, materials[0]));
	entities.push_back(std::make_shared<GameEntity>(triangle, materials[1]));
	entities.push_back(std::make_shared<GameEntity>(square, materials[2]));
	entities.push_back(std::make_shared<GameEntity>(square, materials[0]));
	entities.push_back(std::make_shared<GameEntity>(star, materials[1]));
	entities.push_back(std::make_shared<GameEntity>(star, materials[2]));
	entities.push_back(std::make_shared<GameEntity>(cube, materials[4]));
	entities.push_back(std::make_shared<GameEntity>(cylinder, materials[5]));
	entities.push_back(std::make_shared<GameEntity>(helix, materials[6]));
	entities.push_back(std::make_shared<GameEntity>(quad, materials[7]));
	entities.push_back(std::make_shared<GameEntity>(doubleSidedQuad, materials[8]));
	entities.push_back(std::make_shared<GameEntity>(torus, materials[9]));
	entities.push_back(std::make_shared<GameEntity>(sphere, materials[10]));
	entities.push_back(std::make_shared<GameEntity>(cube, materials[3]));
	entities.push_back(std::make_shared<GameEntity>(cube, materials[10]));
	entities.push_back(std::make_shared<GameEntity>(snowPlane, materials[11]));
	entities.push_back(std::make_shared<GameEntity>(sphere, materials[11]));

	//entity initial transforms
	entities[6]->GetTransform().SetPosition(-9.0f, 0.0f, 0.0f);
	entities[7]->GetTransform().SetPosition(-6.0f, 0.0f, 0.0f);
	entities[8]->GetTransform().SetPosition(-3.0f, 0.0f, 0.0f);
	entities[9]->GetTransform().SetPosition(0.0f, 0.0f, 0.0f);
	entities[9]->GetTransform().SetRotation(-XM_PI / 2, 0.0f, 0.0f);
	entities[10]->GetTransform().SetPosition(3.0f, 0.0f, 0.0f);
	entities[10]->GetTransform().SetRotation(-XM_PI / 2, 0.0f, 0.0f);
	entities[11]->GetTransform().SetPosition(6.0f, 0.0f, 0.0f);
	entities[12]->GetTransform().SetPosition(9.0f, 0.0f, 0.0f);
	entities[13]->GetTransform().SetPosition(0.0f, 0.0f, -20.0f);
	entities[14]->GetTransform().SetPosition(0.0f, -5.0f, 0.0f);
	entities[14]->GetTransform().SetScale(15.0f, 1.0f, 15.0f);
	entities[15]->GetTransform().SetPosition(-45.0f, -3.9f, -15.0f);
	entities[16]->GetTransform().SetPosition(-30.0f, -2.9f, -10.0f);
	entities[16]->GetTransform().SetRotation(XM_PI / 2, 0.0f, 0.0f);

	//make camera
	cameras.push_back(std::make_shared<Camera>(0.0f, 0.0f, -10.0f, 7.5f, 0.02f, XM_PI / 3.0f, (float)this->windowWidth / this->windowHeight, true));
	cameras.push_back(std::make_shared<Camera>(0.0f, 1.0f, -5.0f, 7.5f, 0.02f, XM_PI / 2.0f, (float)this->windowWidth / this->windowHeight, true));
	cameras.push_back(std::make_shared<Camera>(1.0f, -1.0f, -5.0f, 7.5f, 0.02f, XM_PI / 4.0f, (float)this->windowWidth / this->windowHeight, true));
	cameras.push_back(std::make_shared<Camera>(0.0f, 0.0f, -10.0f, 7.5f, 0.02f, XM_PI / 3.0f, (float)this->windowWidth / this->windowHeight, false));

	//set the current active camera
	activeCamera = cameras[0];
}

void Game::CreateAndLoadLights()
{
	//lights
	lights.push_back({});
	lights.push_back({});
	lights.push_back({});
	lights.push_back({});
	lights.push_back({});
	lights.push_back({});
	lights[0].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[0].Direction = { 0.0f, -1.0f, 1.0f };
	lights[0].Color = { 1.0f, 1.0f, 1.0f };
	lights[0].Intensity = 1.0f;
	lights[1].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[1].Direction = { 0.0f, -1.0f, 0.0f };
	lights[1].Color = { 1.0f, 1.0f, 1.0f };
	lights[1].Intensity = 0.1f;
	lights[2].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[2].Direction = { -1.0f, 0.0f, 0.0f };
	lights[2].Color = { 0.0f, 0.0f, 1.0f };
	lights[2].Intensity = 1.0f;
	lights[3].Type = LIGHT_TYPE_POINT;
	lights[3].Position = { -5.0f, 0.0f, -5.0f };
	lights[3].Range = 10.0f;
	lights[3].Color = { 0.0f, 1.0f, 1.0f };
	lights[3].Intensity = 1.0f;
	lights[4].Type = LIGHT_TYPE_POINT;
	lights[4].Position = { 5.0f, 0.0f, 5.0f };
	lights[4].Range = 10.0f;
	lights[4].Color = { 1.0f, 0.0f, 1.0f };
	lights[4].Intensity = 1.0f;
	lights[5].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[5].Direction = { 1.0f, 0.0f, 0.0f };
	lights[5].Color = { 1.0f, 0.0f, 0.0f };
	lights[5].Intensity = 1.0f;
}

void Game::CreateShadowMapResources()
{
	//shadow map resolution and light projection size
	shadowMapResolution = 2048;
	lightProjectionSize = 22.0f;

	//create the shadow map texture
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapResolution;		//should be a power of 2
	shadowDesc.Height = shadowMapResolution;	//should be a power of 2
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	//create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSDesc,
		shadowDSV.GetAddressOf());

	//create shadow map SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(
		shadowTexture.Get(),
		&srvDesc,
		shadowSRV.GetAddressOf());

	//create rasterizer
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000;			//min precision units not world units
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;	//biased of slope
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	//create shadow sampler
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; //only need the first component
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	//light camera matrices
	XMMATRIX lightView = XMMatrixLookToLH(
		XMVectorSet(lights[0].Direction.x, lights[0].Direction.y, lights[0].Direction.z, 0) * -20,	//position 20 units behind the origin
		XMVectorSet(lights[0].Direction.x, lights[0].Direction.y, lights[0].Direction.z, 0),		//direction of the light
		XMVectorSet(0, 1, 0, 0));																	//up is world vector
	XMStoreFloat4x4(&lightViewMatrix, lightView);

	XMMATRIX lightProjection = XMMatrixOrthographicLH(
		lightProjectionSize,
		lightProjectionSize,
		1.0f,
		100.0f);
	XMStoreFloat4x4(&lightProjectionMatrix, lightProjection);
}

void Game::CreateParticleResources()
{
	//depth state
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, particleDepthState.GetAddressOf());

	//additive blend state
	D3D11_BLEND_DESC blend = {};
	blend.AlphaToCoverageEnable = false;
	blend.IndependentBlendEnable = false;
	blend.RenderTarget[0].BlendEnable = true;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blend, particleBlendState.GetAddressOf());

	//particle materials
	std::shared_ptr<Material> snowParticle = std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), particlePixelShader, particleVertexShader);
	snowParticle->AddTextureSRV("Particle", snowSRV);
	snowParticle->AddSampler("BasicSampler", sampler);

	emitters.push_back(std::make_shared<Emitter>(
		1280,								//max particles
		240,								//particles per second
		5.0f,								//life time
		0.1f,								//start size
		0.1f,								//end size
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),	//start color
		XMFLOAT4(1.0f, 1.0f, 1.0f, 0.2f),	//end color
		XMFLOAT3(0, -1, 0),					//start velocity
		XMFLOAT3(0.2f, 0.2f, 0.2f),			//velocity variance
		XMFLOAT3(-30.0, 10.0, 0.0),			//emitter position
		XMFLOAT3(15.0f, 1.0f, 15.0f),		//position variance
		XMFLOAT4(-2, 2, -2, 2),				//rotation variance
		XMFLOAT3(0, -1, 0),					//acceleration
		device,
		snowParticle));
}

void Game::CreatePostProcessResources()
{
	//reset if they exist already
	blurRTV.Reset();
	blurSRV.Reset();

	chromaticRTV.Reset();
	chromaticSRV.Reset();

	//texture description
	D3D11_TEXTURE2D_DESC textureDesc1 = {};
	textureDesc1.Width = windowWidth;
	textureDesc1.Height = windowHeight;
	textureDesc1.ArraySize = 1;
	textureDesc1.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc1.CPUAccessFlags = 0;
	textureDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc1.MipLevels = 1;
	textureDesc1.MiscFlags = 0;
	textureDesc1.SampleDesc.Count = 1;
	textureDesc1.SampleDesc.Quality = 0;
	textureDesc1.Usage = D3D11_USAGE_DEFAULT;

	D3D11_TEXTURE2D_DESC textureDesc2 = {};
	textureDesc2.Width = windowWidth;
	textureDesc2.Height = windowHeight;
	textureDesc2.ArraySize = 1;
	textureDesc2.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc2.CPUAccessFlags = 0;
	textureDesc2.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc2.MipLevels = 1;
	textureDesc2.MiscFlags = 0;
	textureDesc2.SampleDesc.Count = 1;
	textureDesc2.SampleDesc.Quality = 0;
	textureDesc2.Usage = D3D11_USAGE_DEFAULT;

	//create texture resource
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture1;
	device->CreateTexture2D(&textureDesc1, 0, ppTexture1.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture2;
	device->CreateTexture2D(&textureDesc2, 0, ppTexture2.GetAddressOf());

	//create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc1 = {};
	rtvDesc1.Format = textureDesc1.Format;
	rtvDesc1.Texture2D.MipSlice = 0;
	rtvDesc1.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc2 = {};
	rtvDesc2.Format = textureDesc2.Format;
	rtvDesc2.Texture2D.MipSlice = 0;
	rtvDesc2.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(
		ppTexture1.Get(),
		&rtvDesc1,
		chromaticRTV.ReleaseAndGetAddressOf());

	device->CreateRenderTargetView(
		ppTexture2.Get(),
		&rtvDesc2,
		blurRTV.ReleaseAndGetAddressOf());

	//create the Shader Resource View
	//null description gives it the entire resource
	device->CreateShaderResourceView(
		ppTexture1.Get(),
		0,
		chromaticSRV.ReleaseAndGetAddressOf());

	device->CreateShaderResourceView(
		ppTexture2.Get(),
		0,
		blurSRV.ReleaseAndGetAddressOf());
}

// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	//update the cameras projection matrix with the new aspect ratio
	for (int i = 0; i < cameras.size(); i++)
	{
		cameras[i]->UpdateProjectionMatrix((float)this->windowWidth / this->windowHeight);
	}

	CreatePostProcessResources();
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//update ImGui and UI
	ImGuiUpdate(deltaTime);
	BuildUI(_world);

	//movement variables
	float speed = 2.0f;
	float steadySpeed = 0.005f;
	float magnitude = 0.5f;
	float offset = static_cast<float>(sin(totalTime * speed) * magnitude);
	float scaleOffset = static_cast<float>(sin(totalTime * speed) * magnitude + 0.7f);
	static float angle = 0.0f;  //initial angle
	float x = 0.05f * std::cos(angle);  //calculate x position
	float z = 0.05f * std::sin(angle);  //calculate z position
	float dirX = -0.05f * std::sin(angle);
	float dirZ = 0.05f * std::cos(angle);
	float rotationAngle = std::atan2(dirX, dirZ);

	//entity movement
	auto& triangle1 = entities[0]->GetTransform();
	triangle1.Rotate(0.0f, 0.0f, deltaTime);
	auto& triangle2 = entities[1]->GetTransform();
	triangle2.SetScale(scaleOffset, scaleOffset, 1.0f);
	auto& square1 = entities[2]->GetTransform();
	square1.SetPosition(offset, 0.0f, 0.0f);
	auto& square2 = entities[3]->GetTransform();
	square2.SetPosition(0.0f, offset, 0.0f);
	auto& star1 = entities[4]->GetTransform();
	star1.Rotate(0.0f, 0.0f, -deltaTime);
	star1.SetScale(-scaleOffset/2, -scaleOffset/2, 1.0f);
	auto& star2 = entities[5]->GetTransform();
	star2.MoveAbsolute(-0.0001f, 0.0f, 0.0f);
	star2.Scale(1.0001f, 1.0f, 1.0f);

	auto& cube1 = entities[6]->GetTransform();
	cube1.SetScale(scaleOffset, scaleOffset, scaleOffset);
	cube1.Rotate(deltaTime, 0.0f, deltaTime);
	auto& cylinder = entities[7]->GetTransform();
	cylinder.Rotate(0.0f, deltaTime, 0.0f);
	auto& helix = entities[8]->GetTransform();
	helix.Rotate(0.0f, -deltaTime, 0.0f);
	auto& quad = entities[9]->GetTransform();
	quad.Rotate(-deltaTime, 0.0f, -deltaTime);
	auto& doubleSidedQuad = entities[10]->GetTransform();
	doubleSidedQuad.Rotate(0.0f, -deltaTime, 0.0f);
	auto& torus = entities[11]->GetTransform();
	torus.Rotate(0.0f, deltaTime, 0.0f);
	auto& sphere = entities[12]->GetTransform();
	sphere.Rotate(-deltaTime, -deltaTime, -deltaTime);

	auto& cube2 = entities[13]->GetTransform();
	cube2.Rotate(deltaTime, 0.0f, deltaTime);

	auto& snowBall = entities[16]->GetTransform();
	snowBall.MoveAbsolute(x, 0.0f, z);
	snowBall.SetRotation(snowBall.GetPitchYawRoll().x, rotationAngle, snowBall.GetPitchYawRoll().z);
	snowBall.Rotate(0.05f, 0.0f, 0.0f);
	angle += steadySpeed;

	auto& snow = entities[15];
	snow->GetMesh()->UpdateSnow(snowBall.GetPosition().x / 15.0f  + 17.0f, snowBall.GetPosition().z / 15.0f + 15.0, 10.0f);

	//camera update
	activeCamera->Update(deltaTime);

	//reset delta time so a flurry of particles arent released due to build up
	static bool firstFrame = true;
	if (firstFrame) 
	{ 
		deltaTime = 0.0f; 
		firstFrame = false; 
	}

	//update emitters
	if (!firstFrame)
	{
		for (auto& e : emitters)
		{
			e->Update(deltaTime);
		}
	}

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	RenderShadowMaps();

	//post process pre-rendering
	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	context->ClearRenderTargetView(chromaticRTV.Get(), clearColor);
	context->ClearRenderTargetView(blurRTV.Get(), clearColor);
	context->OMSetRenderTargets(1, chromaticRTV.GetAddressOf(), depthBufferDSV.Get()); //change back to chromatic

	//draw all of the entities
	for (auto& entity : entities)
	{
		entity->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightView", lightViewMatrix);
		entity->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightProjection", lightProjectionMatrix);

		entity->GetMaterial()->PrepareMaterial();

		//this is in here so that the UI can update the lights
		for (auto& light : lights)
		{
			entity->GetMaterial()->GetPixelShader()->SetData(
				"lights",								// The name of the (eventual) variable in the shader
				&lights[0],								// The address of the data to set
				sizeof(Light) * (int)lights.size());	// The size of the data (the whole struct!) to set
		}

		entity->GetMaterial()->GetPixelShader()->SetShaderResourceView("ShadowMap", shadowSRV);
		entity->GetMaterial()->GetPixelShader()->SetSamplerState("ShadowSampler", shadowSampler);

		entity->Draw(activeCamera, totalTime);
	}

	//draw sky last
	sky->Draw(activeCamera);

	//set particle state
	context->OMSetBlendState(particleBlendState.Get(), 0, 0xffffffff);	//additive blending
	context->OMSetDepthStencilState(particleDepthState.Get(), 0);	

	//draw emitters
	for (auto& e : emitters)
	{
		e->Draw(context, activeCamera);
	}
	
	//reset states for next frame for particles
	context->OMSetBlendState(0, 0, 0xffffffff);
	context->OMSetDepthStencilState(0, 0);
	context->RSSetState(0);

	//post process post rendering - activate shaders and bind resources, set cbuffer data
	context->OMSetRenderTargets(1, blurRTV.GetAddressOf(), depthBufferDSV.Get());

	ppVertexShader->SetShader();

	chromaticPixelShader->SetShader();
	chromaticPixelShader->SetShaderResourceView("Pixels", chromaticSRV.Get()); // No input needed for chromatic aberration
	chromaticPixelShader->SetSamplerState("ClampSampler", ppSampler.Get());
	chromaticPixelShader->SetFloat3("colorOffset", colorOffset);
	chromaticPixelShader->SetFloat2("screenCenter", DirectX::XMFLOAT2(windowWidth / 2.0f, windowHeight / 2.0f));
	chromaticPixelShader->CopyAllBufferData();

	context->Draw(3, 0); //fullscreen triangle

	//switch back to the back buffer before applying the blur effect
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());

	blurPixelShader->SetShader();
	blurPixelShader->SetShaderResourceView("Pixels", blurSRV.Get()); // Use the output of chromaticPixelShader as input
	blurPixelShader->SetSamplerState("ClampSampler", ppSampler.Get());
	blurPixelShader->SetInt("blurRadius", blurRadius);
	blurPixelShader->SetFloat("pixelWidth", 1.0f / windowWidth);
	blurPixelShader->SetFloat("pixelHeight", 1.0f / windowHeight);
	blurPixelShader->CopyAllBufferData();

	context->Draw(3, 0); //fullscreen triangle

	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

	ID3D11ShaderResourceView* nullSRVs[128] = {};
	context->PSSetShaderResources(0, 128, nullSRVs);

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		bool vsyncNecessary = vsync || !deviceSupportsTearing || isFullscreen;
		swapChain->Present(
			vsyncNecessary ? 1 : 0,
			vsyncNecessary ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}

void Game::RenderShadowMaps()
{
	//reset depth values to 0
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	//unbind back buffer and set the shadow map to current depth buffer
	ID3D11RenderTargetView* nullRTV{};
	context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	//enable shadow rasterizer
	context->RSSetState(shadowRasterizer.Get());

	//unbind pixel shader for shadow rendering
	context->PSSetShader(0, 0, 0);

	//change the viewport to match the shadow map resolution
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)shadowMapResolution;
	viewport.Height = (float)shadowMapResolution;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	//set shadow vertex shaders
	shadowVertexShader->SetShader();
	shadowVertexShader->SetMatrix4x4("view", lightViewMatrix);
	shadowVertexShader->SetMatrix4x4("projection", lightProjectionMatrix);
	//loop and draw all entities
	for (auto& entity : entities)
	{
		shadowVertexShader->SetMatrix4x4("world", entity->GetTransform().GetWorldMatrix());
		shadowVertexShader->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		// Note: Your code may differ significantly here!
		entity->GetMesh()->Draw();
	}

	//reset viewport back to screen
	viewport.Width = (float)this->windowWidth;
	viewport.Height = (float)this->windowHeight;
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(
		1,
		backBufferRTV.GetAddressOf(),
		depthBufferDSV.Get());
	context->RSSetState(0);
}