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

	//helper methods
	void CreateBuffers(Vertex* vertices, int numVertices, UINT* indices, int numIndices);
public:
	//methods
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetIndexCount();
	void Draw();
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

	//constructor (takes in device context, device, vertices, vertex count, indices, & indice count)
	Mesh(Microsoft::WRL::ComPtr<ID3D11DeviceContext> c, 
		Microsoft::WRL::ComPtr<ID3D11Device> d, 
		Vertex* vertices, int numVertices, 
		UINT* indices, int numIndices);
	//constructor for files
	Mesh(const wchar_t* fileName, 
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> c,
		Microsoft::WRL::ComPtr<ID3D11Device> d);
	//destructor
	~Mesh();
};

