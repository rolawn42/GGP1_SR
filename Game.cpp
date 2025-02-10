#include "Game.h"
#include "Graphics.h"
#include "Mesh.h"

#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"

#include "vector"
#include "deque"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

//ImGui includes
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());

	transform = Transform();

	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();
	
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
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);

		//create constant buffer
		D3D11_BUFFER_DESC cbDesc{};
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.ByteWidth = 32;
		cbDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		Graphics::Device->CreateBuffer(&cbDesc, 0, constantBuffer.GetAddressOf()); 

		Graphics::Context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

		//initialize vector data
		colorTint = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		offset = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
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
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

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
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

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
	Vertex vertices1[] =
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
	unsigned int indices1[] = { 0, 1, 2 };

	apmesh_meshes.push_back(std::make_shared<Mesh>(vertices1, indices1, 3, 3));
	++i_meshCount;

	Vertex vertices2[] =
	{
		{ XMFLOAT3(-0.6f, +0.6f, +0.0f), red },
		{ XMFLOAT3(-0.3f, +0.3f, +0.0f), blue },
		{ XMFLOAT3(-0.9f, +0.3f, +0.0f), green },
		{ XMFLOAT3(-0.9f, +0.9f, +0.0f), green },
		{ XMFLOAT3(-0.3f, +0.9f, +0.0f), blue },
	};

	unsigned int indices2[] = { 0, 1, 2, 0, 3, 4 };

	apmesh_meshes.push_back(std::make_shared<Mesh>(vertices2, indices2, 5, 6));
	++i_meshCount;

	Vertex vertices3[] =
	{
		{ XMFLOAT3(+0.0f, +0.9f, +0.0f), red },
		{ XMFLOAT3(+0.4f, +0.7f, +0.0f), green },
		{ XMFLOAT3(+0.2f, +0.7f, +0.0f), blue },
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(-0.4f, +0.7f, +0.0f), green },
		{ XMFLOAT3(-0.2f, +0.7f, +0.0f), blue },

	};

	unsigned int indices3[] = { 0, 1, 2, 1, 3, 2, 4, 5, 3, 5, 4, 0};

	apmesh_meshes.push_back(std::make_shared<Mesh>(vertices3, indices3, 6, 12));
	++i_meshCount;

	/*

	Vertex vertices4[] =
	{
		{ XMFLOAT3(+0.6f, +0.6f, +0.0f), red },
		{ XMFLOAT3(+0.3f, +0.3f, +0.0f), blue },
		{ XMFLOAT3(+0.9f, +0.3f, +0.0f), green },
		{ XMFLOAT3(+0.9f, +0.9f, +0.0f), green },
		{ XMFLOAT3(+0.3f, +0.9f, +0.0f), blue },
	};

	unsigned int indices4[] = { 2, 1, 0, 4, 3, 0 };

	apmesh_meshes.push_back(std::make_shared<Mesh>(vertices4, indices4, 5, 6));
	++i_meshCount;

	*/

	for (unsigned int i = 0; i < i_meshCount; i++) {
		apentity_entities.push_back(std::make_shared<Entity>(apmesh_meshes[i]));
		apentity_entities.push_back(std::make_shared<Entity>(apmesh_meshes[i]));
	}

	i_entityCount = i_meshCount * 2;

	apentity_entities.push_back(std::make_shared<Entity>(apmesh_meshes[2]));
	++i_entityCount;

#pragma region starter explanation
	/*
	// Create a VERTEX BUFFER
	// - This holds the vertex data of triangles for a single object
	// - This buffer is created on the GPU, which is where the data needs to
	//    be if we want the GPU to act on it (as in: draw it to the screen)
	{
		// First, we need to describe the buffer we want Direct3D to make on the GPU
		//  - Note that this variable is created on the stack since we only need it once
		//  - After the buffer is created, this description variable is unnecessary
		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
		vbd.ByteWidth = sizeof(Vertex) * 3;       // 3 = number of vertices in the buffer
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells Direct3D this is a vertex buffer
		vbd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		// Create the proper struct to hold the initial vertex data
		// - This is how we initially fill the buffer with data
		// - Essentially, we're specifying a pointer to the data to copy
		D3D11_SUBRESOURCE_DATA initialVertexData = {};
		initialVertexData.pSysMem = vertices; // pSysMem = Pointer to System Memory

		// Actually create the buffer on the GPU with the initial data
		// - Once we do this, we'll NEVER CHANGE DATA IN THE BUFFER AGAIN
		Graphics::Device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());
	}

	// Create an INDEX BUFFER
	// - This holds indices to elements in the vertex buffer
	// - This is most useful when vertices are shared among neighboring triangles
	// - This buffer is created on the GPU, which is where the data needs to
	//    be if we want the GPU to act on it (as in: draw it to the screen)
	{
		// Describe the buffer, as we did above, with two major differences
		//  - Byte Width (3 unsigned integers vs. 3 whole vertices)
		//  - Bind Flag (used as an index buffer instead of a vertex buffer) 
		D3D11_BUFFER_DESC ibd = {};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
		ibd.ByteWidth = sizeof(unsigned int) * 3;	// 3 = number of indices in the buffer
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
		ibd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		// Specify the initial data for this buffer, similar to above
		D3D11_SUBRESOURCE_DATA initialIndexData = {};
		initialIndexData.pSysMem = indices; // pSysMem = Pointer to System Memory

		// Actually create the buffer with the initial data
		// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
		Graphics::Device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());

	}

	*/

#pragma endregion
}

void Game::UIInfo(float deltaTime) {
	
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
}

bool demoVisibility = true;
bool titleBarViz = true;
bool windowLock = false;
bool styleEditor = false;
float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

int queueSize = 60;
float graphInterval = 0.1f; //how long until the graph gets a new value
float currentWaitTime = 0.0f; //how long has passed since the last new value

float tempOffset[3] = { 0.0f, 0.0f, 0.0f };
float tempTint[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

std::deque<float> af_framerate(queueSize, 0);
std::deque<float> af_frametime(queueSize, 0);

ImGuiWindowFlags next_flags = 0;

void Game::UIUpdate(float deltaTime) {
	ImGuiWindowFlags window_flags = 0;

	ImGui::Begin("Info", 0, next_flags);

	if (ImGui::CollapsingHeader("General")) {

		ImGui::Text("Window Size: %dx%d", Window::Width(), Window::Height());

		if (currentWaitTime >= graphInterval) {
			af_framerate.pop_front();
			af_frametime.pop_front();

			af_framerate.push_back(ImGui::GetIO().Framerate);
			af_frametime.push_back(deltaTime * 1000);

			currentWaitTime = 0;
		}

		currentWaitTime += deltaTime;

		std::string s_framerate = std::to_string(ImGui::GetIO().Framerate) + " fps";
		ImGui::PlotHistogram(s_framerate.c_str(), &af_framerate[0], int(af_framerate.size()), 0, NULL, 0.0f, 5000.0f, ImVec2(0.0f, 50.0f), sizeof(float));

		std::string s_frametime = std::to_string(deltaTime * 1000) + " ms";
		ImGui::PlotHistogram(s_frametime.c_str(), &af_frametime[0], int(af_frametime.size()), 0, NULL, 0.0f, 1.0f, ImVec2(0.0f, 50.0f), sizeof(float));

		ImGui::Text("World Background Color");
		ImGui::ColorEdit4("RGBA color editor", &color[0]);

		ImGui::Checkbox("Demo Window", &demoVisibility);
		ImGui::Checkbox("Title Bar", &titleBarViz);
		ImGui::Checkbox("Lock Window", &windowLock);
		ImGui::Checkbox("Style Editor", &styleEditor);

		ImGui::DragFloat3("Position Offset", &tempOffset[0], 0.05f, -1.5f, 1.5f);
		ImGui::ColorEdit4("Color Tint", &tempTint[0]);

		//ImGui::ColorEdit4("RGBA color editor", &text[0]);
		//ImGui::ColorEdit4("RGBA color editor", &winbg[0]);

		next_flags = window_flags;
	}

	if (ImGui::CollapsingHeader("Meshes")) {
		for (unsigned int i = 0; i < i_meshCount; i++) {
			std::string s_num = "Mesh " + std::to_string(i);
			if (ImGui::CollapsingHeader(s_num.c_str())) {
				ImGui::Text("Tris: %d", (apmesh_meshes[i]->GetIndexCount() / 3));
				ImGui::Text("Verts: %d", (apmesh_meshes[i]->GetVertexCount()));
				ImGui::Text("Indicies: %d", (apmesh_meshes[i]->GetIndexCount()));
			}
		}
	}

	if (ImGui::CollapsingHeader("Entities")) {
		for (unsigned int i = 0; i < i_entityCount; i++) {
			std::string s_num = "Entity " + std::to_string(i);
			if (ImGui::CollapsingHeader(s_num.c_str())) {
				float tempPosition[3] = 
					{ apentity_entities[i]->GetTransform()->GetPosition().x,
					apentity_entities[i]->GetTransform()->GetPosition().y,
					apentity_entities[i]->GetTransform()->GetPosition().z };
				float tempRotation[3] = 
					{ apentity_entities[i]->GetTransform()->GetRotation().x,
					apentity_entities[i]->GetTransform()->GetRotation().y,
					apentity_entities[i]->GetTransform()->GetRotation().z };
				float tempScale[3] = 
					{ apentity_entities[i]->GetTransform()->GetScale().x,
					apentity_entities[i]->GetTransform()->GetScale().y,
					apentity_entities[i]->GetTransform()->GetScale().z };

				std::string s_pos = "Position " + std::to_string(i);
				std::string s_rot = "Rotation " + std::to_string(i);
				std::string s_scl = "Scale " + std::to_string(i);

				ImGui::DragFloat3(s_pos.c_str(), &tempPosition[0], 0.05f, -1.5f, 1.5f);
				ImGui::DragFloat3(s_rot.c_str(), &tempRotation[0], 0.05f, -1.5f, 1.5f);
				ImGui::DragFloat3(s_scl.c_str(), &tempScale[0], 0.05f, -1.5f, 1.5f);

				apentity_entities[i]->GetTransform()->SetPosition(tempPosition[0], tempPosition[1], tempPosition[2]);
				apentity_entities[i]->GetTransform()->SetRotation(tempRotation[0], tempRotation[1], tempRotation[2]);
				apentity_entities[i]->GetTransform()->SetScale(tempScale[0], tempScale[1], tempScale[2]);
			}
		}
	}

	if (demoVisibility)
		ImGui::ShowDemoWindow();
	if (styleEditor)
		ImGui::ShowStyleEditor();
	if (windowLock) {
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
	}
	if (!titleBarViz)
		window_flags |= ImGuiWindowFlags_NoTitleBar;

	ImGui::End();

	//colorTint = XMFLOAT4(tempTint[0], tempTint[1], tempTint[2], tempTint[3]);
	//offset = XMFLOAT4(tempOffset[0], tempOffset[1], tempOffset[2], 0.0f);
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{

}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------


void Game::Update(float deltaTime, float totalTime)
{
	//ui
	UIInfo(deltaTime);
	UIUpdate(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();


	transform.SetPosition(sin(totalTime), 0, 0);
	transform.Rotate(0, 0, deltaTime);

	float s = sin(totalTime * 20) * 0.5f + 1.0f;
	transform.SetScale(s, s, s);

	apentity_entities[0]->GetTransform()->SetPosition(transform.GetPosition().x, transform.GetPosition().y, transform.GetPosition().z);
	apentity_entities[0]->GetTransform()->SetRotation(transform.GetRotation().x, transform.GetRotation().y, transform.GetRotation().z);
	apentity_entities[0]->GetTransform()->SetScale(transform.GetScale().x, transform.GetScale().y, transform.GetScale().z);
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
		// Clear the back buffer (erase what's on screen) and depth buffer
		//const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

#pragma region old constant buffer code

	/*
	XMMATRIX rotZMat = XMMatrixRotationZ(totalTime);
	XMMATRIX trMat = XMMatrixTranslation(sin(totalTime), 0, 0);

	XMFLOAT4X4 tr;
	XMFLOAT4X4 rotZ;
	XMStoreFloat4x4(&tr, trMat);
	XMStoreFloat4x4(&rotZ, rotZMat);
	*/

	/*
	//collect the data locally
	VertexShaderData dataToCopy{};
	dataToCopy.colorTint = colorTint;
	dataToCopy.transform = transform.GetWorldMatrix();

	D3D11_MAPPED_SUBRESOURCE mapped{};
	Graphics::Context->Map(
		constantBuffer.Get(),
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mapped
	);

	unsigned int size = sizeof(VertexShaderData);
	size = ((size + 15) / 16) * 16;

	memcpy(mapped.pData, &dataToCopy, size);

	Graphics::Context->Unmap(constantBuffer.Get(), 0);

	Graphics::Context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we setting right now?
		constantBuffer.GetAddressOf()); // Array of buffers (or address of just one)
	*/

#pragma endregion

	for (unsigned int i = 0; i < i_entityCount; i++) { 
		apentity_entities[i]->Draw();
	}

#pragma region create geo explanation
	/*
	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		// Set buffers in the input assembler (IA) stage
		//  - Do this ONCE PER OBJECT, since each object may have different geometry
		//  - For this demo, this step *could* simply be done once during Init()
		//  - However, this needs to be done between EACH DrawIndexed() call
		//     when drawing different geometry, so it's here as an example
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		Graphics::Context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		Graphics::Context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Tell Direct3D to draw
		//  - Begins the rendering pipeline on the GPU
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all currently set Direct3D resources (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		Graphics::Context->DrawIndexed(
			3,     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices
	}
	*/

#pragma endregion
	 
	//prepares ImGUI buffers and uses them to draw on screen
	{
		ImGui::Render(); // Turns this frame�s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
	}

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}



