#include "Camera.h"
#include "Input.h"
#include <algorithm>

using namespace DirectX;

Camera::Camera(float _x, float _y, float _z, float mSpeed, float lSpeed, float fov, float aspectRatio, bool pO) :
	nearPlane(0.01),
	farPlane(1000)
{
	transform.SetPosition(_x, _y, _z);
	moveSpeed = mSpeed;
	lookSpeed = lSpeed;
	FOV = fov;
	perspOrtho = pO;

	//initialize starting matrices
	XMStoreFloat4x4(&viewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&projMatrix, XMMatrixIdentity());

	//update the matrices
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

//gets the view matrix
DirectX::XMFLOAT4X4 Camera::GetView()
{
	return viewMatrix;
}

//gets the projection matrix
DirectX::XMFLOAT4X4 Camera::GetProjection()
{
	return projMatrix;
}

Transform Camera::GetTransform()
{
	return transform;
}

float Camera::GetFOV()
{
	return FOV;
}

bool Camera::GetType()
{
	return perspOrtho;
}

//updates the projection matrix and stores it
void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	//if it perpective store the perspective matrix
	if (perspOrtho)
	{
		XMStoreFloat4x4(&projMatrix, XMMatrixPerspectiveFovLH(FOV, aspectRatio, nearPlane, farPlane));
	}
	//otherwise determine the halfwidth/height and store the orthographic matrix
	else
	{
		float halfWidth = 10.0f;
		float halfHeight = halfWidth / aspectRatio;

		XMStoreFloat4x4(&projMatrix, XMMatrixOrthographicLH(halfWidth, halfHeight, nearPlane, farPlane));
	}
}

//updates the view matrix and stores it
void Camera::UpdateViewMatrix()
{
	//position of the camera
	XMFLOAT3 pos = transform.GetPosition();
	//direction its looking
	XMFLOAT3 fwd = transform.GetForward();
	//make the view matrix with global up vector
	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&pos), XMLoadFloat3(&fwd), XMVectorSet(0, 1, 0, 0));
	//store view matrix
	XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::Update(float dt)
{
	//reference to the input manager's instance
	Input& input = Input::GetInstance();

	//WASD relative controls
	if (input.KeyDown('W')) { transform.MoveRelative(0.0, 0.0, dt * moveSpeed); }
	if (input.KeyDown('S')) { transform.MoveRelative(0.0, 0.0, dt * -moveSpeed); }
	if (input.KeyDown('A')) { transform.MoveRelative(dt * -moveSpeed, 0.0, 0.0); }
	if (input.KeyDown('D')) { transform.MoveRelative(dt * moveSpeed, 0.0, 0.0); }

	//up and down absolute controls
	if (input.KeyDown(' ')) { transform.MoveAbsolute(0.0, dt * moveSpeed, 0.0); }
	if (input.KeyDown(VK_LCONTROL)) { transform.MoveAbsolute(0.0, dt * -moveSpeed, 0.0); }

	//mouse movement
	if (input.MouseLeftDown())
	{
		float cursorMovementX = input.GetMouseXDelta();
		float cursorMovementY = input.GetMouseYDelta();

		cursorMovementX *= lookSpeed;
		cursorMovementY *= lookSpeed;

		//apply the rotation
		transform.Rotate(cursorMovementY, cursorMovementX, 0);

		//clamp the rotation to prevent flipping upside down
		XMFLOAT3 currentRotation = transform.GetPitchYawRoll();
		XMVECTOR rotationVector = XMLoadFloat3(&currentRotation);
		//min and max
		XMVECTOR minRotation = XMVectorSet(-XM_PIDIV2, -XM_PIDIV2, -XM_PIDIV2, -XM_PIDIV2);
		XMVECTOR maxRotation = XMVectorSet(XM_PIDIV2, XM_PIDIV2, XM_PIDIV2, XM_PIDIV2);
		//clamping the X only
		XMVECTOR clampedRotation = XMVectorClamp(rotationVector, minRotation, maxRotation);
		clampedRotation = DirectX::XMVectorSetY(clampedRotation, DirectX::XMVectorGetY(rotationVector));
		
		//store and set the rotation
		XMStoreFloat3(&currentRotation, clampedRotation);
		transform.SetRotation(currentRotation);
	}

	//update the view matrix
	UpdateViewMatrix();
}
