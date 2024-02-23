#pragma once
#include "Input.h"
#include "Transform.h"
#include <DirectXMath.h>

class Camera
{
public:
	//constructor
	Camera(float _x, float _y, float _z, float mSpeed, float lSpeed, float fov, float aspectRatio, bool pO);

	//getters
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	Transform GetTransform();
	float GetFOV();
	bool GetType();

	//update methods
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt);
private:
	Transform transform;

	//matrices
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	//camera variables
	float moveSpeed;
	float lookSpeed;
	float FOV;
	float nearPlane;
	float farPlane;
	bool perspOrtho;
};

