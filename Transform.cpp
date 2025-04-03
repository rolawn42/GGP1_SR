#include "Transform.h"

using namespace DirectX;

Transform::Transform() :
    position(0, 0, 0),
    pitchYawRoll(0, 0, 0),
    scale(1, 1, 1),
    up(0, 1, 0),
    right(1, 0, 0),
    forward(0, 0, 1),
    matriciesDirty(false),
    vectorsDirty(false)
{
    XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());
    XMStoreFloat4x4(&worldITMatrix, DirectX::XMMatrixIdentity());
}

Transform::Transform(float x, float y, float z) :
    pitchYawRoll(0, 0, 0),
    scale(1, 1, 1),
    up(0, 1, 0),
    right(1, 0, 0),
    forward(0, 0, 1),
    matriciesDirty(false),
    vectorsDirty(false)
{
    SetPosition(x, y, z);

    XMStoreFloat4x4(&worldMatrix, DirectX::XMMatrixIdentity());
    XMStoreFloat4x4(&worldITMatrix, DirectX::XMMatrixIdentity());
}

Transform::~Transform()
{
}

void Transform::SetPosition(float x, float y, float z)
{
    position.x = x;
    position.y = y;
    position.z = z;
    matriciesDirty = true;
}

void Transform::SetPosition(DirectX::XMFLOAT3 pos)
{
    SetPosition(pos.x, pos.y, pos.z);
}

void Transform::SetRotation(float p, float y, float r)
{
    pitchYawRoll.x = p;
    pitchYawRoll.y = y;
    pitchYawRoll.z = r;
    matriciesDirty = true;
}

void Transform::SetRotation(DirectX::XMFLOAT3 rot)
{
    SetRotation(rot.x, rot.y, rot.z);
}

void Transform::SetScale(float x, float y, float z)
{
    scale.x = x;
    scale.y = y;
    scale.z = z;
    matriciesDirty = true;
}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
    SetScale(scale.x, scale.y, scale.x);
}

void Transform::MoveAbsolute(float x, float y, float z)
{
    DirectX::XMStoreFloat3(&position, XMLoadFloat3(&position) + DirectX::XMVectorSet(x, y, z, 0.0f));
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 vector)
{
    MoveAbsolute(vector.x, vector.y, vector.z);
}

void Transform::MoveRelative(float x, float y, float z)
{
    //move along  our "local axes

    //create a vector from the params
    XMVECTOR movement = XMVectorSet(x, y, z, 0.0f);

    //create a quaternion for rotation based on our p/y/r
    XMVECTOR rotQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));

    //perform rotation of our desired movement
    XMVECTOR dir = XMVector3Rotate(movement, rotQuat);

    //add this "rotated direction" to our position
    DirectX::XMStoreFloat3(&position, XMLoadFloat3(&position) + dir);
    vectorsDirty = true;

}

void Transform::MoveRelative(DirectX::XMFLOAT3 vector)
{
    MoveRelative(vector.x, vector.y, vector.z);
}

void Transform::Rotate(float p, float y, float r)
{
    pitchYawRoll.x += p;
    pitchYawRoll.y += y;
    pitchYawRoll.z += r;
    matriciesDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
    scale.x *= x;
    scale.y *= y;
    scale.z *= z;
    matriciesDirty = true;
}

DirectX::XMFLOAT3 Transform::GetPosition() { return position; }

DirectX::XMFLOAT3 Transform::GetRotation() { return pitchYawRoll; }

DirectX::XMFLOAT3 Transform::GetScale() { return scale; }

DirectX::XMFLOAT3 Transform::GetRight() { 
    XMFLOAT3 temp;
    XMStoreFloat3(&temp, XMVector3Rotate(XMLoadFloat3(&right), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll))));

    return temp; 
}

DirectX::XMFLOAT3 Transform::GetUp() { 
    XMFLOAT3 temp;
    XMStoreFloat3(&temp, XMVector3Rotate(XMLoadFloat3(&up), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll))));

    return up; 
}

DirectX::XMFLOAT3 Transform::GetForward() { 
    XMFLOAT3 temp;
    XMStoreFloat3(&temp, XMVector3Rotate(XMLoadFloat3(&forward), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll))));

    return temp; 
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix() 
{ 
    if (matriciesDirty) {
        //make translation, rotation, and scale matricies

        DirectX::XMMATRIX trMatrix = DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&position)); //XMMatrixTranslation(position.x, position.y, position.z)
        DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
        DirectX::XMMATRIX scMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

        DirectX::XMMATRIX world = scMatrix * rotMatrix * trMatrix;

        XMStoreFloat4x4(&worldMatrix, world);
        XMStoreFloat4x4(&worldITMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));

        // the world matrix is a combination of translation, rotation, and scale matricies

        matriciesDirty = false;
    }

    return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
    GetWorldMatrix();

    return worldITMatrix;
}
