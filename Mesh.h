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

public:
	//methods
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();
	void Draw(ID3D11DeviceContext* context);

	//constructor
	Mesh(ID3D11Device* device, Vertex* vertices, int numVertices, int* indices, int numIndices);
	//destructor
	~Mesh();
};

