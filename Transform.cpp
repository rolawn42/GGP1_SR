#include "Transform.h"

using namespace DirectX;

Transform::Transform() : 
    position(0, 0, 0), 
    pitchYawRoll(0, 0, 0), 
    scale(1, 1, 1),
    dirty(false)
{
    XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());
}

Transform::~Transform()
{
}

void Transform::SetPosition(float x, float y, float z)
{
    position.x = x;
    position.y = y;
    position.z = z;
    dirty = true;
}

void Transform::SetRotation(float p, float y, float r)
{
    pitchYawRoll.x = p;
    pitchYawRoll.y = y;
    pitchYawRoll.z = r;
    dirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
    scale.x = x;
    scale.y = y;
    scale.z = z;
    dirty = true;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
    XMStoreFloat3(&position, XMLoadFloat3(&position) + DirectX::XMVectorSet(x, y, z, 0.0f));
}

void Transform::Rotate(float p, float y, float z)
{
    pitchYawRoll.x += p;
    pitchYawRoll.y += y;
    pitchYawRoll.z += z;
    dirty = true;
}

void Transform::Scale(float x, float y, float z)
{
    scale.x += x;
    scale.y += y;
    scale.z += z;
    dirty = true;
}

DirectX::XMFLOAT3 Transform::GetPosition() { return position; }

DirectX::XMFLOAT3 Transform::GetRotation() { return pitchYawRoll; }

DirectX::XMFLOAT3 Transform::GetScale() { return scale; }

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix() 
{ 
    if (dirty) {
        //make translation, rotation, and scale matricies

        DirectX::XMMATRIX trMatrix = DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&position)); //XMMatrixTranslation(position.x, position.y, position.z)
        DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
        DirectX::XMMATRIX scMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

        XMStoreFloat4x4(&worldMatrix, scMatrix * rotMatrix * trMatrix);

        // the world matrix is a combination of translation, rotation, and scale matricies

        return worldMatrix;

        dirty = false;
    }
}
