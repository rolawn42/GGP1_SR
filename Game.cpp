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
	ambientColor = DirectX::XMFLOAT3(0.25f, 0.2f, 0.225f);

	CreateDirectional(1.0f, XMFLOAT3(1, 0, 0), XMFLOAT3(1, 0, 0));
	CreatePoint(1.0f, XMFLOAT3(1, 1, 1), 6.0f, XMFLOAT3(-6.0f, 0.0f, 0.0f));
	CreatePoint(1.0f, XMFLOAT3(0, 0, 1), 6.0f, XMFLOAT3(-3.0f, 0.0f, 0.0f));
	CreatePoint(1.0f, XMFLOAT3(1, 0, 0), 6.0f, XMFLOAT3(0.0f, 0.0f, 0.0f));
	CreatePoint(1.0f, XMFLOAT3(0, 0, 1), 6.0f, XMFLOAT3(3.0f, 0.0f, 0.0f));
	CreateSpot(10.0f, XMFLOAT3(1, 0, 0), XMFLOAT3(0, -1, 0), 6.0f, XMFLOAT3(7.5f, 3.5f, 0.0f), XMConvertToRadians(20.0f), XMConvertToRadians(30.0f));
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
	
	sky = std::make_shared<Sky>(samplerState, meshes[0], vss[1], pss[1]);

	//create SRV
	sky->CreateCubemap(
		FIXPATH(L"../../Assets/Skies/CloudsPink/CloudsPink_Right.png"),
		FIXPATH(L"../../Assets/Skies/CloudsPink/CloudsPink_Left.png"),
		FIXPATH(L"../../Assets/Skies/CloudsPink/CloudsPink_Up.png"),
		FIXPATH(L"../../Assets/Skies/CloudsPink/CloudsPink_Down.png"),
		FIXPATH(L"../../Assets/Skies/CloudsPink/CloudsPink_Front.png"),
		FIXPATH(L"../../Assets/Skies/CloudsPink/CloudsPink_Back.png"));
	
	//CREATE SRVS (essentially TEXTURES)

	//Denim
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> denimBCSRV;
	MAKESRV(denimBCSRV, FIXPATH(L"../../Assets/Textures/Denim/Denim_BC.png"));

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> denimNSRV;
	MAKESRV(denimNSRV, FIXPATH(L"../../Assets/Textures/Denim/Denim_N.png"));

	//Cobblestone
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brickBCSRV;
	MAKESRV(brickBCSRV, FIXPATH(L"../../Assets/Textures/Cobblestone/Cobblestone_BC.png"));

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brickNSRV;
	MAKESRV(brickNSRV, FIXPATH(L"../../Assets/Textures/Cobblestone/Cobblestone_N.png"));

	//Cusion
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionBCSRV;
	MAKESRV(cushionBCSRV, FIXPATH(L"../../Assets/Textures/Cushion/Cushion_BC.png"));

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cushionNSRV;
	MAKESRV(cushionNSRV, FIXPATH(L"../../Assets/Textures/Cushion/Cushion_N.png"));


	//CREATE MATERIALS

	materials.push_back(std::make_shared<Material>("Denim Normal", XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vss[0], pss[0], 0.8f));
	materials[0]->AddTextureSRV("SurfaceTexture", denimBCSRV);
	materials[0]->AddTextureSRV("NormalMap", denimNSRV);
	materials[0]->AddSampler("BasicSampler", samplerState);

	materials.push_back(std::make_shared<Material>("Denim Brown", XMFLOAT4(0.8f, 0.5f, 0.0f, 1.0f), vss[0], pss[0], 0.4f));
	materials[1]->AddTextureSRV("SurfaceTexture", denimBCSRV);
	materials[1]->AddTextureSRV("NormalMap", denimNSRV);
	materials[1]->AddSampler("BasicSampler", samplerState);

	materials.push_back(std::make_shared<Material>("Bricks Normal", XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vss[0], pss[0], 1.0f));
	materials[2]->AddTextureSRV("SurfaceTexture", brickBCSRV);
	materials[2]->AddTextureSRV("NormalMap", brickNSRV);
	materials[2]->AddSampler("BasicSampler", samplerState);

	materials.push_back(std::make_shared<Material>("Cushion", XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), vss[0], pss[0], 0.2f, XMFLOAT2{1.5f, 1.5f}));
	materials[3]->AddTextureSRV("SurfaceTexture", cushionBCSRV);
	materials[3]->AddTextureSRV("NormalMap", cushionNSRV);
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
	UIUpdate(deltaTime, currentCamera, cameras, meshes, entities, materials, lights);

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
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), backgroundColor);
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
