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
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		context->VSSetShader(vertexShader.Get(), 0, 0);
		context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	//bind buffer to pipeline
	context->VSSetConstantBuffers(
		0,								//which slot to bind the buffer to
		1,								//how many buffers being set
		constBuffer.GetAddressOf());	//buffers in question

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	ImGui::StyleColorsDark();

	//push all the entities
	entities.push_back(std::make_shared<GameEntity>(triangle));
	entities.push_back(std::make_shared<GameEntity>(triangle));
	entities.push_back(std::make_shared<GameEntity>(square));
	entities.push_back(std::make_shared<GameEntity>(square));
	entities.push_back(std::make_shared<GameEntity>(star));
	entities.push_back(std::make_shared<GameEntity>(star));

	//make camera
	cameras.push_back(std::make_shared<Camera>(0.0, 0.0, -1.0, 1, 1, XM_PI / 2,(float)this->windowWidth / this->windowHeight, true)); //normal camera
	//cameras.push_back(std::make_shared<Camera>(0.0, 10.0, -1.0, 1, 1, 90.0, this->windowWidth, this->windowHeight, true)); //90 fov above
	//cameras.push_back(std::make_shared<Camera>(10.0, -10.0, -1.0, 1, 1, 45.0, this->windowWidth, this->windowHeight, true)); //45 fov to the right and below
	//cameras.push_back(std::make_shared<Camera>(0.0, 0.0, -1.0, 1, 1, 60.0, this->windowWidth, this->windowHeight, false)); //orthographic
	activeCamera = cameras[0];
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
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	//create constant buffer
	unsigned int size = sizeof(VertexShaderData);	//get size from vertex shader data
	size = (size + 15) / 16 * 16;					//calculate size in multiple of 16
	D3D11_BUFFER_DESC cbDesc = {};					//sets struct to all zeros
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	//will be used by a constant buffer
	cbDesc.ByteWidth = size;						//must be a multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//C++ will be writing but not reading data to this after creation
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;				//dynamic buffer allowing it to be changed

	device->CreateBuffer(&cbDesc, 0, constBuffer.GetAddressOf());

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red	= XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green	= XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue	= XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 yellow = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

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
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	int triIndices[] = { 0, 1, 2 };

	int numTriVerts = sizeof(triVerts) / sizeof(triVerts[0]);
	int numTriIndices = sizeof(triIndices) / sizeof(triIndices[0]);

	//create triangle mesh
	triangle = std::make_shared<Mesh>(context, device, triVerts, numTriVerts, triIndices, numTriIndices);

	//square
	Vertex squareVerts[] =
	{
		{ XMFLOAT3(-0.8f, +0.8f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, +0.8f, +0.0f), red },
		{ XMFLOAT3(-0.8f, +0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, +0.5f, +0.0f), red },
	};

	int squareIndices[] = { 0, 1, 2, 2, 1, 3 };

	int numSquareVerts = sizeof(squareVerts) / sizeof(squareVerts[0]);
	int numSquareIndices = sizeof(squareIndices) / sizeof(squareIndices[0]);

	square = std::make_shared<Mesh>(context, device, squareVerts, numSquareVerts, squareIndices, numSquareIndices);

	//star
	Vertex starVerts[] =
	{
		{ XMFLOAT3(+0.7f, +0.6f, +0.0f), white },
		{ XMFLOAT3(+0.7f, +0.45f, +0.0f), yellow },
		{ XMFLOAT3(+0.55f, +0.3f, +0.0f), yellow },
		{ XMFLOAT3(+0.6f, +0.55f, +0.0f), yellow },
		{ XMFLOAT3(+0.5f, +0.7f, +0.0f), yellow },
		{ XMFLOAT3(+0.65f, +0.7f, +0.0f), yellow },
		{ XMFLOAT3(+0.7f, +0.95f, +0.0f), yellow },
		{ XMFLOAT3(+0.75f, +0.7f, +0.0f), yellow },
		{ XMFLOAT3(+0.9f, +0.7f, +0.0f), yellow },
		{ XMFLOAT3(+0.8f, +0.55f, +0.0f), yellow },
		{ XMFLOAT3(+0.85f, +0.3f, +0.0f), yellow },
	};

	int starIndices[] = { 1, 2, 0, 0, 2, 3, 3, 4, 0, 0, 4, 5, 5, 6, 0, 0, 6, 7, 7, 8, 0, 0, 8, 9, 9, 10, 0, 0, 10, 1 };

	int numStarVerts = sizeof(starVerts) / sizeof(starVerts[0]);
	int numStarIndices = sizeof(starIndices) / sizeof(starIndices[0]);

	star = std::make_shared<Mesh>(context, device, starVerts, numStarVerts, starIndices, numStarIndices);
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
void Game::BuildUI(float color[4], DirectX::XMFLOAT4& tint, DirectX::XMFLOAT4X4& world)
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

	//vertex shader changers
	ImGui::ColorEdit4("Tint Color", &tint.x);

	//demo window visibility button to show and hide the demo window
	if (ImGui::Button("ImGui Demo Window"))
	{
		ImGuiDemoVisable = !ImGuiDemoVisable;
	}
	if (ImGuiDemoVisable)
	{
		ImGui::ShowDemoWindow();
	}

	//check box example
	ImGui::Checkbox("Show/Hide", &ImGuiDemoVisable);

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
	BuildUI(bgColor, _colorTint, _world);

	//movement variables
	float speed = 1.0f;
	float magnitude = 1.0f;
	float offset = static_cast<float>(sin(totalTime * speed) * magnitude);
	float scaleOffset = static_cast<float>(sin(totalTime * speed) * magnitude + 1.0);

	//entity movement
	auto& entity1 = entities[0]->GetTransform();
	entity1.Rotate(0.0f, 0.0f, deltaTime);
	auto& entity2 = entities[1]->GetTransform();
	entity2.SetScale(scaleOffset, scaleOffset, 0.0f);
	auto& entity3 = entities[2]->GetTransform();
	entity3.SetPosition(offset, 0.0f, 0.0f);
	auto& entity4 = entities[3]->GetTransform();
	entity4.SetPosition(0.0f, offset, 0.0f);
	auto& entity5 = entities[4]->GetTransform();
	entity5.Rotate(0.0f, 0.0f, -deltaTime);
	entity5.SetScale(-scaleOffset/2, -scaleOffset/2, 0.0f);
	auto& entity6 = entities[5]->GetTransform();
	entity6.MoveAbsolute(-0.0001f, 0.0f, 0.0f);
	entity6.Scale(1.0001f, 1.0f, 1.0f);

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
		entity->Draw(context, constBuffer, activeCamera, _colorTint);
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