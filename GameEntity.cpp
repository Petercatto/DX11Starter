#include "GameEntity.h"

//constructor that saves a mesh and material ptr to a mesh and material
GameEntity::GameEntity(std::shared_ptr<Mesh> meshPtr, std::shared_ptr<Material> matPtr)
{
	mesh = meshPtr;
	material = matPtr;
}

//returns the mesh of the entity
std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

//returns the transform of the entity
Transform& GameEntity::GetTransform()
{
	return transform;
}

//returns the material of the entity
std::shared_ptr<Material> GameEntity::GetMaterial()
{
	return material;
}

//sets the material of the entity
void GameEntity::SetMaterial(std::shared_ptr<Material> matPtr)
{
	material = matPtr;
}

//method that draws entities
void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera, float totalTime)
{
	//get the world matrix from the transform
	DirectX::XMFLOAT4X4 worldMatrix = transform.GetWorldMatrix();

	std::shared_ptr<SimpleVertexShader> vsData = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> psData = material->GetPixelShader();
	psData->SetFloat4("colorTint", material->GetColor());			// Strings here MUST
	psData->SetFloat("totalTime", totalTime);
	vsData->SetMatrix4x4("world", transform.GetWorldMatrix());		// match variable
	vsData->SetMatrix4x4("view", camera->GetView());				// names in your
	vsData->SetMatrix4x4("projection", camera->GetProjection());	// shader’s cbuffer!

	//mapping constant buffer data
	vsData->CopyAllBufferData();
	psData->CopyAllBufferData();

	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	//draw the mesh
	mesh->Draw();
}
