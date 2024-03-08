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

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();

	//make materials
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f, pixelShader, vertexShader));	//dull red
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), 0.5f, pixelShader, vertexShader));	//somewhat shiny green
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), 1.0f, pixelShader, vertexShader));	//shiny blue
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, customShader, vertexShader));	//custom pixel shader
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f, pixelShader, vertexShader));	//dull white
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f, pixelShader, vertexShader));	//somewhat shiny white
	materials.push_back(std::make_shared<Material>(DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, pixelShader, vertexShader));	//shiny white

	//push all the entities
	entities.push_back(std::make_shared<GameEntity>(triangle, materials[0]));
	entities.push_back(std::make_shared<GameEntity>(triangle, materials[1]));
	entities.push_back(std::make_shared<GameEntity>(square, materials[2]));
	entities.push_back(std::make_shared<GameEntity>(square, materials[0]));
	entities.push_back(std::make_shared<GameEntity>(star, materials[1]));
	entities.push_back(std::make_shared<GameEntity>(star, materials[2]));
	entities.push_back(std::make_shared<GameEntity>(cube, materials[4]));
	entities.push_back(std::make_shared<GameEntity>(cylinder, materials[4]));
	entities.push_back(std::make_shared<GameEntity>(helix, materials[4]));
	entities.push_back(std::make_shared<GameEntity>(quad, materials[4]));
	entities.push_back(std::make_shared<GameEntity>(doubleSidedQuad, materials[4]));
	entities.push_back(std::make_shared<GameEntity>(torus, materials[4]));
	entities.push_back(std::make_shared<GameEntity>(sphere, materials[4]));
	entities.push_back(std::make_shared<GameEntity>(cube, materials[3]));

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

	//make camera
	cameras.push_back(std::make_shared<Camera>(0.0f, 0.0f, -10.0f, 7.5f, 0.02f, XM_PI / 3.0f, (float)this->windowWidth / this->windowHeight, true));
	cameras.push_back(std::make_shared<Camera>(0.0f, 1.0f, -5.0f, 7.5f, 0.02f, XM_PI / 2.0f, (float)this->windowWidth / this->windowHeight, true));
	cameras.push_back(std::make_shared<Camera>(1.0f, -1.0f, -5.0f, 7.5f, 0.02f, XM_PI / 4.0f, (float)this->windowWidth / this->windowHeight, true));
	cameras.push_back(std::make_shared<Camera>(0.0f, 0.0f, -10.0f, 7.5f, 0.02f, XM_PI / 3.0f, (float)this->windowWidth / this->windowHeight, false));

	//set the current active camera
	activeCamera = cameras[0];

	//lights
	lights.push_back({});
	lights.push_back({});
	lights.push_back({});
	lights.push_back({});
	lights.push_back({});
	lights[0].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[0].Direction = { 1.0f, 0.0f, 0.0f };
	lights[0].Color = { 1.0f, 0.0f, 0.0f };
	lights[0].Intensity = 1.0f;
	lights[1].Type = LIGHT_TYPE_DIRECTIONAL;
	lights[1].Direction = { 0.0f, -1.0f, 0.0f };
	lights[1].Color = { 0.0f, 1.0f, 0.0f };
	lights[1].Intensity = 1.0f;
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
void Game::BuildUI(float color[4], DirectX::XMFLOAT4X4& world)
{
	//beginning of window
	ImGui::Begin((std::string(nameUI) + "'s Window").c_str());

	//shows the current frame rate
	ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);

	//shows the current window size
	ImGui::Text("Window Resolution: %dx%d", windowWidth, windowHeight);
	
	//text input example
	ImGui::InputText("Enter Name", textInput, IM_ARRAYSIZE(textInput));

	//copies the contents of text input into the name UI
	if (ImGui::Button("Change Window Name"))
	{
		strncpy_s(nameUI, textInput, IM_ARRAYSIZE(nameUI));
	}

	//background color picker which takes in the bgcolor which is now a public variable
	ImGui::ColorEdit4("Background Color", &color[0]);
	//update ambientcolor
	ambientColor = { color[0] / 4,color[1] / 4,color[2] / 4 };

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

	//info for the current camera
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

	//ending of the window
	ImGui::End();
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
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//update ImGui and UI
	ImGuiUpdate(deltaTime);
	BuildUI(bgColor, _world);

	//movement variables
	float speed = 2.0f;
	float magnitude = 0.5f;
	float offset = static_cast<float>(sin(totalTime * speed) * magnitude);
	float scaleOffset = static_cast<float>(sin(totalTime * speed) * magnitude + 0.7f);

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

	//camera update
	activeCamera->Update(deltaTime);

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

	//draw all of the entities
	for (auto& entity : entities)
	{
		//update lighting
		entity->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
		for (auto& light : lights)
		{
			entity->GetMaterial()->GetPixelShader()->SetData(
				"lights",								// The name of the (eventual) variable in the shader
				&lights[0],								// The address of the data to set
				sizeof(Light) * (int)lights.size());	// The size of the data (the whole struct!) to set
		}

		entity->Draw(context, activeCamera, totalTime);
	}

	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

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