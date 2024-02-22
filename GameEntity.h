#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include <d3d11.h>
#include "BufferStructs.h"
#include "Camera.h"

class GameEntity
{
public:
	//constructor
	GameEntity(std::shared_ptr<Mesh> meshPtr);

	//getters
	std::shared_ptr<Mesh> GetMesh();
	Transform& GetTransform();

	//draw method
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Microsoft::WRL::ComPtr<ID3D11Buffer> cBuffer, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4 tint);
private:
	//transform and mesh data
	Transform transform;
	std::shared_ptr<Mesh> mesh;
};

