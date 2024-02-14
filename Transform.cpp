#include "Transform.h"

using namespace DirectX;

//constructor with starting values at the origin, no rotation and 1 scale
Transform::Transform() :
	position(0, 0, 0),
	rotation(0, 0, 0),
	scale(1, 1, 1),
	worldMatrixUpdated(false)
{
	//create an identity matrix and store it to both the world and world inverse matrices
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
}

//set position via x y z values
void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;

	worldMatrixUpdated = true;
}

//set position via vector
void Transform::SetPosition(DirectX::XMFLOAT3 _position)
{
	position.x = _position.x;
	position.y = _position.y;
	position.z = _position.z;

	worldMatrixUpdated = true;
}

//set rotation via pitch, yaw, roll values
void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation.x = pitch;
	rotation.y = yaw;
	rotation.z = roll;

	worldMatrixUpdated = true;
}

//set rotation via vector
void Transform::SetRotation(DirectX::XMFLOAT3 _rotation)
{
	rotation.x = _rotation.x;
	rotation.y = _rotation.y;
	rotation.z = _rotation.z;

	worldMatrixUpdated = true;
}

//set scale via x y z values
void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;

	worldMatrixUpdated = true;
}

//set scale via vector
void Transform::SetScale(DirectX::XMFLOAT3 _scale)
{
	scale.x = _scale.x;
	scale.y = _scale.y;
	scale.z = _scale.z;

	worldMatrixUpdated = true;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if(worldMatrixUpdated) updateWorldMatrix();
	worldMatrixUpdated = false;
	return world;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	if (worldMatrixUpdated) updateWorldMatrix();
	worldMatrixUpdated = false;
	return worldInverseTranspose;
}

//method to update the world matrix
void Transform::updateWorldMatrix()
{
	//create the three matrices that make up the world matrix
	XMMATRIX s = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
	XMMATRIX r = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation));
	XMMATRIX t = XMMatrixTranslationFromVector(XMLoadFloat3(&position));

	//combine into a single world matrix
	XMMATRIX worldMat = s * r * t;
	XMStoreFloat4x4(&world, worldMat);
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(worldMat)));
}

//translate absolute via x y z values
void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;

	worldMatrixUpdated = true;
}

//translate absolute via vector
void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	position.x += offset.x;
	position.y += offset.y;
	position.z += offset.z;

	worldMatrixUpdated = true;
}

//rotate via pitch yaw roll values
void Transform::Rotate(float pitch, float yaw, float roll)
{
	rotation.x += pitch;
	rotation.y += yaw;
	rotation.z += roll;

	worldMatrixUpdated = true;
}

//rotate via vector
void Transform::Rotate(DirectX::XMFLOAT3 _rotation)
{
	rotation.x += _rotation.x;
	rotation.y += _rotation.y;
	rotation.z += _rotation.z;

	worldMatrixUpdated = true;
}

//scale via x y z values
void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;

	worldMatrixUpdated = true;
}

//scale via vector
void Transform::Scale(DirectX::XMFLOAT3 _scale)
{
	scale.x *= _scale.x;
	scale.y *= _scale.y;
	scale.z *= _scale.z;

	worldMatrixUpdated = true;
}
