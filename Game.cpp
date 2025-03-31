#include "Game.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

//ImGui
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

//Macros
#define FIXPATH(x) FixPath(x).c_str()
#define UISTEP 0.05f

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

	//CAMERA

	for (unsigned int i = 0; i < 2; i++) {
		cameras.push_back(std::make_shared<Camera>(
			XMFLOAT3(0, 0, -15), 4.0f, 0.005f, XM_PIDIV4, Window::AspectRatio()));
	}

	currentCamera = cameras[0];

	//Generates light sources (directional, point, spot) and sets the ambient color
	CreateLighting();

	//Creates meshes, materials, and entities
	CreateGeometry();

	Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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


void Game::CreateLighting()
{
	ambientColor = DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f);

	CreateDirectional(1.0f, XMFLOAT3(1, 0, 0), XMFLOAT3(1, 0, 0));
	CreatePoint(1.0f, XMFLOAT3(0, 0, 0), 6.0f, XMFLOAT3(-6.0f, 0.0f, 0.0f));
	CreatePoint(1.0f, XMFLOAT3(0, 0, 1), 6.0f, XMFLOAT3(-3.0f, 0.0f, 0.0f));
	CreatePoint(1.0f, XMFLOAT3(1, 0, 0), 6.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
	CreatePoint(1.0f, XMFLOAT3(1, 0, 0), 6.0f, XMFLOAT3(3.0f, 0.0f, 0.0f));
	CreateSpot(1.0f, XMFLOAT3(1, 0, 0), XMFLOAT3(0, -1, 0), 6.0f, XMFLOAT3(7.5f, 2.5f, 0.0f), XMConvertToRadians(10.0f), XMConvertToRadians(15.0f));
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
/*
inline void MakeSRV(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv, const wchar_t* texFile) {
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		texFile,
		0,
		srv.GetAddressOf());
}
*/

void Game::CreateGeometry()
{

	//CREATE SAMPLER STATE
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // What happens outside the 0-1 uv range?
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;		// How do we handle sampling "between" pixels?
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	Graphics::Device->CreateSamplerState(&sampDesc, samplerState.GetAddressOf());

	//CREATE MESHES

	meshes.push_back(std::make_shared<Mesh>("Cube", FIXPATH("../../Assets/Models/cube.obj")));
	meshes.push_back(std::make_shared<Mesh>("Cylinder", FIXPATH("../../Assets/Models/cylinder.obj")));
	meshes.push_back(std::make_shared<Mesh>("Helix", FIXPATH("../../Assets/Models/helix.obj")));
	meshes.push_back(std::make_shared<Mesh>("Sphere", FIXPATH("../../Assets/Models/sphere.obj")));
	meshes.push_back(std::make_shared<Mesh>("Torus", FIXPATH("../../Assets/Models/torus.obj")));
	meshes.push_back(std::make_shared<Mesh>("Quad_Double", FIXPATH("../../Assets/Models/quad_double_sided.obj")));
	meshes.push_back(std::make_shared<Mesh>("Quad", FIXPATH("../../Assets/Models/quad.obj")));

	//CREATE SHADERS

	vss.push_back(std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FIXPATH(L"VertexShader.cso")));
	vss.push_back(std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FIXPATH(L"VertexShader_Sky.cso")));

	pss.push_back(std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FIXPATH(L"PixelShader.cso")));
	pss.push_back(std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FIXPATH(L"PixelShader_Sky.cso")));

	//CREATE SKYBOX
	#define MAKESRV(srv, texFile) DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), texFile, 0, srv.GetAddressOf());
	
	sky = std::make_shared<Sky>(samplerState, meshes[0]);

	//create SRV
	sky->CreateCubemap(
		FIXPATH(L"../../Assets/Textures/Denim/Denim_BC.png"),
		FIXPATH(L"../../Assets/Textures/Denim/Denim_BC.png"),
		FIXPATH(L"../../Assets/Textures/Denim/Denim_BC.png"),
		FIXPATH(L"../../Assets/Textures/Denim/Denim_BC.png"),
		FIXPATH(L"../../Assets/Textures/Denim/Denim_BC.png"),
		FIXPATH(L"../../Assets/Textures/Denim/Denim_BC.png"));

	//setShaders
	sky->SetVertexShader(vss[1]);
	sky->SetPixelShader(pss[1]);
	
	//CREATE SRVS (essentially TEXTURES)

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> denimBCSRV;
	MAKESRV(denimBCSRV, FIXPATH(L"../../Assets/Textures/Denim/Denim_BC.png"));

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brickBCSRV;
	MAKESRV(brickBCSRV, FIXPATH(L"../../Assets/Textures/MixedBrick/MixedBrick_BC.png"));

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> denimNSRV;
	MAKESRV(denimNSRV, FIXPATH(L"../../Assets/Textures/Decal/Decal_N.png"));

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brickNSRV;
	MAKESRV(brickNSRV, FIXPATH(L"../../Assets/Textures/MixedBrick/MixedBrick_N.png"));

	//CREATE MATERIALS

	materials.push_back(std::make_shared<Material>("Denim Normal", XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vss[0], pss[0]));
	materials[0]->AddTextureSRV("SurfaceTexture", denimBCSRV);
	materials[0]->AddTextureSRV("NormalMap", denimNSRV);
	materials[0]->AddSampler("BasicSampler", samplerState);

	materials.push_back(std::make_shared<Material>("Denim Brown", XMFLOAT4(0.8f, 0.5f, 0.0f, 1.0f), vss[0], pss[0], 0.0f));
	materials[1]->AddTextureSRV("SurfaceTexture", denimBCSRV);
	materials[1]->AddTextureSRV("NormalMap", denimNSRV);
	materials[1]->AddSampler("BasicSampler", samplerState);

	materials.push_back(std::make_shared<Material>("Bricks Normal", XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vss[0], pss[0], 1.0f));
	materials[2]->AddTextureSRV("SurfaceTexture", brickBCSRV);
	materials[2]->AddTextureSRV("NormalMap", brickNSRV);
	materials[2]->AddSampler("BasicSampler", samplerState);

	materials.push_back(std::make_shared<Material>("Bricks Small", XMFLOAT4(0.3f, 0.6f, 0.2f, 1.0f), vss[0], pss[0]));
	materials[3]->AddTextureSRV("SurfaceTexture", brickBCSRV);
	materials[3]->AddTextureSRV("NormalMap", brickNSRV);
	materials[3]->AddSampler("BasicSampler", samplerState);

	//CREATE ENTITIES

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
	entities.push_back(std::make_shared<Entity>(meshes[6], materials[2], std::make_shared<Transform>(+4.5f, -2.0f, 0.0f)));
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
	float color[4] = { 0.05f, 0.05f, 0.05f, 0.0f };

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

		next_flags = window_flags;
	}

	if (ImGui::CollapsingHeader("Camera")) {
		for (unsigned int i = 0; i < cameras.size(); i++) {
			ImGui::PushID(cameras[i].get());
			std::string camName = "Camera " + std::to_string(i);
			if (ImGui::CollapsingHeader(camName.c_str())) {
				ImGui::Text("Current?: %s", cameras[i] == currentCamera ? "yes" : "no");
				
				if (ImGui::Button("Make Current?")) {
					currentCamera = cameras[i];
				}

				std::shared_ptr<Transform> transform = cameras[i]->GetTransform();

				XMFLOAT3 tempPos = transform->GetPosition();
				XMFLOAT3 tempRot = transform->GetRotation();

				if (ImGui::DragFloat3("Position", &tempPos.x, UISTEP)) { transform->SetPosition(tempPos); }
				if (ImGui::DragFloat3("Rotation", &tempRot.x, UISTEP)) { transform->SetRotation(tempRot); }
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Lights")) {
		for (unsigned int i = 0; i < lights.size(); i++) {
			ImGui::PushID(i);
			if (ImGui::CollapsingHeader(LightType(lights[i].type))) {
				ImGui::DragFloat("Intensity", &lights[i].intensity, UISTEP);
				ImGui::ColorEdit3("Color", &lights[i].color.x);
				if (lights[i].type == LIGHT_TYPE_DIRECTIONAL || lights[i].type == LIGHT_TYPE_SPOT) {
					ImGui::DragFloat3("Direction", &lights[i].direction.x, UISTEP);
				}
				if (lights[i].type == LIGHT_TYPE_POINT || lights[i].type == LIGHT_TYPE_SPOT) {
					ImGui::DragFloat3("Position", &lights[i].position.x, UISTEP);
					ImGui::DragFloat("Range", &lights[i].range, UISTEP);
				}
				if (lights[i].type == LIGHT_TYPE_SPOT) {
					float innerTemp = DirectX::XMConvertToDegrees(lights[i].spotInnerAngle);
					float outerTemp = DirectX::XMConvertToDegrees(lights[i].spotOuterAngle);

					if (ImGui::DragFloat("Inner Cone Angle", &innerTemp, UISTEP)) { lights[i].spotInnerAngle = XMConvertToRadians(innerTemp); }
					if (ImGui::DragFloat("Outer Cone Angle", &outerTemp, UISTEP)) { lights[i].spotOuterAngle = XMConvertToRadians(outerTemp); }
				}
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Entities")) {
		for (unsigned int i = 0; i < entities.size(); i++) {
			ImGui::PushID(entities[i].get());
			if (ImGui::CollapsingHeader("Entity")) {
				std::shared_ptr<Transform> transform = entities[i]->GetTransform();

				DirectX::XMFLOAT3 tempPosition = transform->GetPosition();
				DirectX::XMFLOAT3 tempRotation = transform->GetRotation();
				DirectX::XMFLOAT3 tempScale = transform->GetScale();

				if (ImGui::DragFloat3("Position " + i, &tempPosition.x, UISTEP, -1.5f, 1.5f)) { transform->SetPosition(tempPosition); }
				if (ImGui::DragFloat3("Rotation " + i, &tempRotation.x, UISTEP, -1.5f, 1.5f)) { transform->SetRotation(tempRotation); }
				if (ImGui::DragFloat3("Scale " + i, &tempScale.x, UISTEP, -1.5f, 1.5f)) { transform->SetScale(tempScale); }
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Materials")) {

		for (unsigned int i = 0; i < materials.size(); i++) {
			if (ImGui::CollapsingHeader(materials[i]->GetName())) {
				ImGui::PushID(materials[i].get());
				DirectX::XMFLOAT4 tempTint = materials[i]->GetColorTint();
				DirectX::XMFLOAT2 tempScale = materials[i]->GetUvScale();
				DirectX::XMFLOAT2 tempOffset = materials[i]->GetUvOffset();
				float roughness = materials[i]->GetRoughness();

				if(ImGui::DragFloat3("Tint", &tempTint.x, UISTEP)) { materials[i]->SetColorTint(tempTint);  }
				if(ImGui::DragFloat2("Scale", &tempScale.x, UISTEP)) { materials[i]->SetUvScale(tempScale);  }
				if(ImGui::DragFloat2("Offset", &tempOffset.x, UISTEP)) { materials[i]->SetUvOffset(tempOffset); }
				if(ImGui::DragFloat("Roughness", &roughness, UISTEP)) { materials[i]->SetRoughness(roughness); }

				for (auto& [name, ptr] : materials[i]->GetTextureSRVMap()) {
					ImGui::Text(name.c_str());
					ImGui::Image((ImTextureID)(intptr_t)ptr.Get(), ImVec2(256, 256));
				}
				ImGui::PopID();
			}
		}
	}


	if (ImGui::CollapsingHeader("Meshes")) {
		for (unsigned int i = 0; i < meshes.size(); i++) {
			if (ImGui::CollapsingHeader(meshes[i]->GetName())) {
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

DirectX::XMFLOAT4 Game::GetColorTint()
{
	return colorTint;
}

Transform Game::GetTransform()
{
	return transform;
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
		entities[i]->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
		entities[i]->GetMaterial()->GetPixelShader()->SetInt("lightCount", (int)lights.size());
		entities[i]->GetMaterial()->GetPixelShader()->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		entities[i]->Draw(currentCamera);
	}

	sky->Draw(currentCamera);
	 
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

//LIGHTING HELPERS

void Game::CreateDirectional(float intensity, DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 direction)
{
	CreateLight(0, intensity, color, direction, 0.0f, DirectX::XMFLOAT3(), 0.0f, 0.0f);
}
void Game::CreatePoint(float intensity, DirectX::XMFLOAT3 color, float range, DirectX::XMFLOAT3 position)
{
	CreateLight(1, intensity, color, DirectX::XMFLOAT3(), range, position, 0.0f, 0.0f);
}
void Game::CreateSpot(float intensity, DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 direction, float range, DirectX::XMFLOAT3 position, float spotInnerAngle, float spotOuterAngle)
{
	CreateLight(2, intensity, color, direction, range, position, spotInnerAngle, spotOuterAngle);
}
void Game::CreateLight(int type, float intensity, DirectX::XMFLOAT3 color, DirectX::XMFLOAT3 direction, float range, DirectX::XMFLOAT3 position, float spotInnerAngle, float spotOuterAngle)
{
	Light light = { };
	light.type = type;
	light.direction = direction;
	light.color = color;
	light.intensity = intensity;
	light.position = position;
	light.range = range;
	light.spotInnerAngle = spotInnerAngle;
	light.spotOuterAngle = spotOuterAngle;

	lights.push_back(light);
}

const char* Game::LightType(int num)
{
	switch(num) {
		case 0:
			return "Directional";
		case 1:
			return "Point";
		case 2:
			return "Spot";
		default:
			return "NULL";
	}
}
