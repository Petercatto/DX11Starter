#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include <d3d11.h>
#include "Camera.h"
#include "Material.h"

class GameEntity
{
public:
	//constructor
	GameEntity(std::shared_ptr<Mesh> meshPtr, std::shared_ptr<Material> matPtr);

	//getters
	std::shared_ptr<Mesh> GetMesh();
	Transform& GetTransform();
	std::shared_ptr<Material> GetMaterial();

	//setters
	void SetMaterial(std::shared_ptr<Material> matPtr);

	//draw method
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera, float totalTime);
private:
	//transform, mesh and material data
	Transform transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};

