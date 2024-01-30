#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include "DXCore.h"
#include "Vertex.h"

class Mesh
{
private:
	//buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	//holds the number of indices a mesh contains
	int indexCount = 0;
	//device context
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<ID3D11Device> device;

public:
	//methods
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();
	void Draw();

	//constructor (takes in device context, device, vertices, vertex count, indices, & indice count)
	Mesh(Microsoft::WRL::ComPtr<ID3D11DeviceContext> c, Microsoft::WRL::ComPtr<ID3D11Device> d, Vertex* vertices, int numVertices, int* indices, int numIndices);
	//destructor
	~Mesh();
};

