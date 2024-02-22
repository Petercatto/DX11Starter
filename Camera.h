#pragma once
#include "Input.h"
#include "Transform.h"
#include <DirectXMath.h>

class Camera
{
public:
	//constructor
	Camera(float _x, float _y, float _z, float mSpeed, float lSpeed, float fov, float aR, bool pO);

	//getters
	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();

	//update methods
	void UpdateProjectionMatrix(float aR);
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
	float aspectRatio;
	float FOV;
	float nearPlane;
	float farPlane;
	bool perspOrtho;
};

