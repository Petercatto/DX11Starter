#include "GameEntity.h"

//constructor that saves a mesh ptr to the mesh
GameEntity::GameEntity(std::shared_ptr<Mesh> meshPtr) :
	mesh(meshPtr)
{

}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

Transform& GameEntity::GetTransform()
{
	return transform;
}

//method that draws entities
void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, Microsoft::WRL::ComPtr<ID3D11Buffer> cBuffer, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4 tint)
{
	//get the world matrix from the transform
	DirectX::XMFLOAT4X4 worldMatrix = transform.GetWorldMatrix();

	//update the constant buffer with the world matrix
	VertexShaderData vsData;
	vsData.colorTint = tint;
	vsData.world = worldMatrix;
	vsData.view = camera->GetView();
	vsData.projection = camera->GetProjection();

	//map the constant buffer resource
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	context->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	//copy the data to the constant buffer
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

	//unmap the constant vuffer resource
	context->Unmap(cBuffer.Get(), 0);

	//draw the mesh
	mesh->Draw();
}
