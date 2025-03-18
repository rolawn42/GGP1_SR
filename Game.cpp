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

	// ImGui style
	ImGui::StyleColorsDark();

	for (unsigned int i = 0; i < 2; i++) {
		cameras.push_back(std::make_shared<Camera>(
			//CHANGED: here I moved the camera farther back, in Camera.cpp I implemented the position
			//value in the constructor, which I had forgotten to do previously (also I increased move speed)
			XMFLOAT3(0, 0, -15), 4.0f, 0.005f, XM_PIDIV4, Window::AspectRatio()));
	}

	currentCamera = cameras[0];
	
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateGeometry();

	Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); //CHANGED: this line was added (as you know)
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
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	//CHANGED: replaced the 'quad' (one sided) with the torus
	meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Models/cube.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Models/cylinder.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Models/helix.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Models/sphere.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Models/torus.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>(FixPath("../../Assets/Models/quad_double_sided.obj").c_str()));

	vss.push_back(std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str()));

	//CHANGED: these were all named wrong (I changed the names in the explorer and forgot to rename them here
	pss.push_back(std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str()));
	pss.push_back(std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"DebugUVsPS.cso").c_str()));
	pss.push_back(std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"DebugNormalsPS.cso").c_str()));
	pss.push_back(std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"CustomPS.cso").c_str()));

	//materials.push_back(std::make_shared<Material>(XMFLOAT4(1, 0, 0, 0), vss[0], pss[0]));
	//materials.push_back(std::make_shared<Material>(XMFLOAT4(0, 1, 0, 0), vss[0], pss[0]));
	//materials.push_back(std::make_shared<Material>(XMFLOAT4(0, 0, 1, 0), vss[0], pss[0]));

	//CHANGED: all the below are changed just for organizing the meshes in scene
	materials.push_back(std::make_shared<Material>(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), vss[0], pss[0]));
	materials.push_back(std::make_shared<Material>(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), vss[0], pss[1]));
	materials.push_back(std::make_shared<Material>(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), vss[0], pss[2]));
	materials.push_back(std::make_shared<Material>(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), vss[0], pss[3]));

	entities.push_back(std::make_shared<Entity>(meshes[0], materials[0], std::make_shared<Transform>(-7.5f, +2.0f, 0.0f)));
	entities.push_back(std::make_shared<Entity>(meshes[1], materials[1], std::make_shared<Transform>(-4.5f, +2.0f, 0.0f)));
	entities.push_back(std::make_shared<Entity>(meshes[2], materials[0], std::make_shared<Transform>(-1.5f, +2.0f, 0.0f)));
	entities.push_back(std::make_shared<Entity>(meshes[3], materials[1], std::make_shared<Transform>(+1.5f, +2.0f, 0.0f)));
	entities.push_back(std::make_shared<Entity>(meshes[4], materials[0], std::make_shared<Transform>(+4.5f, +2.0f, 0.0f)));
	entities.push_back(std::make_shared<Entity>(meshes[5], materials[1], std::make_shared<Transform>(+7.5f, +2.0f, 0.0f)));

	entities.push_back(std::make_shared<Entity>(meshes[0], materials[2], std::make_shared<Transform>(-7.5f, -2.0f, 0.0f)));
	entities.push_back(std::make_shared<Entity>(meshes[1], materials[3], std::make_shared<Transform>(-4.5f, -2.0f, 0.0f)));
	entities.push_back(std::make_shared<Entity>(meshes[2], materials[2], std::make_shared<Transform>(-1.5f, -2.0f, 0.0f)));
	entities.push_back(std::make_shared<Entity>(meshes[3], materials[3], std::make_shared<Transform>(+1.5f, -2.0f, 0.0f)));
	entities.push_back(std::make_shared<Entity>(meshes[4], materials[2], std::make_shared<Transform>(+4.5f, -2.0f, 0.0f)));
	entities.push_back(std::make_shared<Entity>(meshes[5], materials[3], std::make_shared<Transform>(+7.5f, -2.0f, 0.0f)));
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

//temp solution for global vars issue
namespace {

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

}

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

	if (ImGui::CollapsingHeader("Camera")) {
		for (unsigned int i = 0; i < cameras.size(); i++) {
			std::string s_num = "Camera " + std::to_string(i);
			if (ImGui::CollapsingHeader(s_num.c_str())) {
				ImGui::Text("Current?: %s", cameras[i] == currentCamera ? "yes" : "no");
				std::string s_num = "Make Current " + std::to_string(i);
				if (ImGui::Button(s_num.c_str())) {
					currentCamera = cameras[i];
				}
				//CHANGED: I think this would have been part of A6, but either way I incorrectly wrote the code for writing out the camera 
				//position and rotation, I used %d and did not split the position to XYZ (like cameras[i]->GT()->GP()), I used it this time to debug
				ImGui::Text("Postion: X: %f Y: %f Z: %f", 
					(cameras[i]->GetTransform()->GetPosition().x), 
					(cameras[i]->GetTransform()->GetPosition().y), 
					(cameras[i]->GetTransform()->GetPosition().z));
				ImGui::Text("Rotation: P: %f Y: %f R: %f", 
					(cameras[i]->GetTransform()->GetRotation().x),
					(cameras[i]->GetTransform()->GetRotation().y),
					(cameras[i]->GetTransform()->GetRotation().z));
			}
		}
	}

	if (ImGui::CollapsingHeader("Entities")) {
		for (unsigned int i = 0; i < entities.size(); i++) {
			std::string s_num = "Entity " + std::to_string(i);
			if (ImGui::CollapsingHeader(s_num.c_str())) {
				float tempPosition[3] = 
					{ entities[i]->GetTransform()->GetPosition().x,
					entities[i]->GetTransform()->GetPosition().y,
					entities[i]->GetTransform()->GetPosition().z };
				float tempRotation[3] = 
					{ entities[i]->GetTransform()->GetRotation().x,
					entities[i]->GetTransform()->GetRotation().y,
					entities[i]->GetTransform()->GetRotation().z };
				float tempScale[3] = 
					{ entities[i]->GetTransform()->GetScale().x,
					entities[i]->GetTransform()->GetScale().y,
					entities[i]->GetTransform()->GetScale().z };

				std::string s_pos = "Position " + std::to_string(i);
				std::string s_rot = "Rotation " + std::to_string(i);
				std::string s_scl = "Scale " + std::to_string(i);

				ImGui::DragFloat3(s_pos.c_str(), &tempPosition[0], 0.05f, -1.5f, 1.5f);
				ImGui::DragFloat3(s_rot.c_str(), &tempRotation[0], 0.05f, -1.5f, 1.5f);
				ImGui::DragFloat3(s_scl.c_str(), &tempScale[0], 0.05f, -1.5f, 1.5f);

				entities[i]->GetTransform()->SetPosition(tempPosition[0], tempPosition[1], tempPosition[2]);
				entities[i]->GetTransform()->SetRotation(tempRotation[0], tempRotation[1], tempRotation[2]);
				entities[i]->GetTransform()->SetScale(tempScale[0], tempScale[1], tempScale[2]);
			}
		}
	}

	if (ImGui::CollapsingHeader("Meshes")) {
		for (unsigned int i = 0; i < meshes.size(); i++) {
			std::string s_num = "Mesh " + std::to_string(i);
			if (ImGui::CollapsingHeader(s_num.c_str())) {
				ImGui::Text("Tris: %d", (meshes[i]->GetIndexCount() / 3));
				ImGui::Text("Verts: %d", (meshes[i]->GetVertexCount()));
				ImGui::Text("Indicies: %d", (meshes[i]->GetIndexCount()));
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
	if (currentCamera)
		currentCamera->UpdateProjectionMatrix(Window::AspectRatio());
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

	currentCamera->Update(deltaTime);

	//transform.SetPosition(sin(static_cast<double>(totalTime)), 0, 0);
	//transform.Rotate(0, 0, deltaTime);

	//float s = sin(static_cast<double>(totalTime * 20)) * 0.5f + 1.0f;
	//transform.SetScale(s, s, s);
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

	for (unsigned int i = 0; i < entities.size(); i++) { 
		entities[i]->Draw(currentCamera);
	}
	 
	//prepares ImGUI buffers and uses them to draw on screen
	{
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
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



