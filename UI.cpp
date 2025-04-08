#include "UI.h"

#define UISTEP 0.05f

using namespace DirectX;

//variables
namespace {

	bool demoVisibility = false;
	bool titleBarViz = true;
	bool windowLock = false;
	bool styleEditor = false;

	int queueSize = 60;
	float graphInterval = 0.1f; //how long until the graph gets a new value
	float currentWaitTime = 0.0f; //how long has passed since the last new value

	std::deque<float> af_framerate(queueSize, 0);
	std::deque<float> af_frametime(queueSize, 0);

	ImGuiWindowFlags next_flags = 0;
}

void UIInfo(float deltaTime) {

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

void UIUpdate(float deltaTime,
	std::shared_ptr<Camera> currentCamera, std::vector<std::shared_ptr<Camera>> cameras,
	std::vector<std::shared_ptr<Mesh>> meshes,
	std::vector<std::shared_ptr<Entity>> entities,
	std::vector<std::shared_ptr<Material>> materials, 
	std::vector<Light> lights) {

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
			if (ImGui::TreeNode(camName.c_str())) {
				ImGui::Text("Current?: %s", cameras[i] == currentCamera ? "yes" : "no");

				if (ImGui::Button("Make Current?")) {
					currentCamera = cameras[i];
				}

				std::shared_ptr<Transform> transform = cameras[i]->GetTransform();

				DF3("Position", transform->GetPosition(), [&](XMFLOAT3 x) { transform->SetPosition(x); });
				DF3("Rotation", transform->GetRotation(), [&](XMFLOAT3 x) { transform->SetRotation(x); });

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Lights")) {
		for (unsigned int i = 0; i < lights.size(); i++) {
			ImGui::PushID(i);
			if (ImGui::TreeNode(LightType(lights[i].type))) {
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
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Entities")) {
		for (unsigned int i = 0; i < entities.size(); i++) {
			ImGui::PushID(entities[i].get());
			if (ImGui::TreeNode("Entity")) {
				std::shared_ptr<Transform> transform = entities[i]->GetTransform();

				DF3("Position", transform->GetPosition(), [&](XMFLOAT3 x) { transform->SetPosition(x); });
				DF3("Rotation", transform->GetRotation(), [&](XMFLOAT3 x) { transform->SetRotation(x); });
				DF3("Scale", transform->GetScale(), [&](XMFLOAT3 x) { transform->SetScale(x); });

				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Materials")) {

		for (unsigned int i = 0; i < materials.size(); i++) {
			ImGui::PushID(materials[i].get());
			if (ImGui::TreeNode(materials[i]->GetName())) {

				#define GETCOLORTINT materials[i]->GetColorTint()
				DF3("Tint", XMFLOAT3(GETCOLORTINT.x, GETCOLORTINT.y, GETCOLORTINT.z), [&](XMFLOAT3 x) { materials[i]->SetColorTint3(x); });
				DF2("Scale", materials[i]->GetUvScale(), [&](XMFLOAT2 x) { materials[i]->SetUvScale(x); });
				DF2("Offset", materials[i]->GetUvOffset(), [&](XMFLOAT2 x) { materials[i]->SetUvOffset(x); });
				DF1("Roughness", materials[i]->GetRoughness(), [&](float x) { materials[i]->SetRoughness(x); });

				for (auto& [name, ptr] : materials[i]->GetTextureSRVMap()) {
					ImGui::Text(name.c_str());
					ImGui::Image((ImTextureID)(intptr_t)ptr.Get(), ImVec2(256, 256));
				}

				//DragPosition(x)
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Meshes")) {
		for (unsigned int i = 0; i < meshes.size(); i++) {
			if (ImGui::TreeNode(meshes[i]->GetName())) {
				ImGui::Text("Tris: %d", (meshes[i]->GetIndexCount() / 3));
				ImGui::Text("Verts: %d", (meshes[i]->GetVertexCount()));
				ImGui::Text("Indicies: %d", (meshes[i]->GetIndexCount()));

				ImGui::TreePop();
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

//Drag Float 1
void DF1(const char* name, float startValue, std::function<void(float)> endLocation) {
	if (ImGui::DragFloat(name, &startValue, UISTEP)) { endLocation(startValue); }
}


//Drag Float 2
void DF2(const char* name, XMFLOAT2 startValue, std::function<void(XMFLOAT2)> endLocation) {
	if (ImGui::DragFloat2(name, &startValue.x, UISTEP)) { endLocation(startValue); }
}


//Drag Float 3
void DF3(const char* name, XMFLOAT3 startValue, std::function<void(XMFLOAT3)> endLocation) {
	if (ImGui::DragFloat3(name, &startValue.x, UISTEP)) { endLocation(startValue); }
}

const char* LightType(int num)
{
	switch (num) {
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