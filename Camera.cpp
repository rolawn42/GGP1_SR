#include "Camera.h"

using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 pos, float moveSpeed, float lookSpeed, float fov, float aspectRatio) :
    moveSpeed(moveSpeed),
    mouseLookSpeed(lookSpeed),
    fov(fov),
    aspectRatio(aspectRatio)
{
    //CHANGED: just added the provided position to the transform
    transform = std::make_shared<Transform>(pos.x, pos.y, pos.z);

    UpdateViewMatrix();
    UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{
}

void Camera::Update(float dt) 
{
    float speed = dt * moveSpeed;
    
    if (Input::KeyDown('W')) { transform->MoveRelative(0, 0, speed); }
    if (Input::KeyDown('S')) { transform->MoveRelative(0, 0, -speed); }
    if (Input::KeyDown('A')) { transform->MoveRelative(-speed, 0, 0); }
    if (Input::KeyDown('D')) { transform->MoveRelative(speed, 0, 0); }
    if (Input::KeyDown('Q')) { transform->MoveAbsolute(0, speed, 0); }
    if (Input::KeyDown('E')) { transform->MoveAbsolute(0, -speed, 0); }

    if (Input::MouseLeftDown()) {
        //how much did it move since last frame
        //Input::GetMouseXDelta();
        //Input::GetMouseYDelta();

        float xRot = mouseLookSpeed * Input::GetMouseXDelta();
        float yRot = mouseLookSpeed * Input::GetMouseYDelta();

        if (xRot != 0.0f || yRot != 0.0f) {
            transform->Rotate(yRot, xRot, 0);

            XMFLOAT3 rot = transform->GetRotation();

            if (rot.x > XM_PIDIV2) rot.x = XM_PIDIV2 - 0.0001f;
            if (rot.x < -XM_PIDIV2) rot.x = -XM_PIDIV2 + 0.0001f;
            transform->SetRotation(rot);
        }
    }

    UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
    XMFLOAT3 pos = transform->GetPosition();
    XMFLOAT3 fwd = transform->GetForward();
    XMFLOAT3 worldUp = XMFLOAT3(0, 1, 0);
    
    XMMATRIX view = XMMatrixLookToLH(
        XMVectorSet(pos.x, pos.y, pos.z, 1.0f), 
        XMVectorSet(fwd.x, fwd.y, fwd.z, 1.0f), 
        XMVectorSet(worldUp.x, worldUp.y, worldUp.z, 1.0f));

    XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
    //call the dxmath function to make a perspective projection
    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XM_PIDIV4, //FOV angle in radians
        aspectRatio,
        0.1f,
        1000.0f);
    XMStoreFloat4x4(&projectionMatrix, proj);
}

DirectX::XMFLOAT4X4 Camera::GetView() { return viewMatrix; }

DirectX::XMFLOAT4X4 Camera::GetProjection() { return projectionMatrix; }

std::shared_ptr<Transform> Camera::GetTransform() { return transform; }
