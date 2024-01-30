#include "Mesh.h"

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer()
{
	return vertexBuffer;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer()
{
	return indexBuffer;
}

int Mesh::GetIndexCount()
{
	return indexCount;
}

void Mesh::Draw(ID3D11DeviceContext* context)
{
	//draw geometry
	//steps are repeated for each object
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

	//set buffers in the input assembler (IA) stage
    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    //tell direct3D to draw
	//begins the rendering pipeline on the GPU
	//uses current direct3D resources such as shaders, buffers, etc
	//DrawIndexed() uses the index buffer to look up corresponding vertices in the vertex buffer
    context->DrawIndexed(
        indexCount,	//number of indices to use
        0,			//offset to the first index
        0);			//offset to add to each index when looking up vertices
}


Mesh::Mesh(ID3D11Device* device, Vertex* vertices, int numVertices, int* indices, int numIndices)
{
	//initialize indexCount
	indexCount = numIndices;

	//create a vertex buffer
	//holds the vertex data of triangles for a single object
	//buffer is created on the GPU which is where the data needs to go
	{
		//describe the buffer we want direct3D to make on the GPU
		//this variable is created on the stack since we only need it once
		//after the buffer is created this description variable is unnecessary
		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_IMMUTABLE;				//doesn't change
		vbd.ByteWidth = sizeof(Vertex) * numVertices;	//number of vertices
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;		//tells Direct3D this is a vertex buffer
		vbd.CPUAccessFlags = 0;							//cannot access the data from C++
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		//create the proper struct to hold the initial vertex data
		D3D11_SUBRESOURCE_DATA initialVertexData = {};
		initialVertexData.pSysMem = vertices;	//pointer to system memory

		//create the buffer on the GPU with the initial data
		device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());
	}

	//create a index buffer
	//holds indices to elements in the vertex buffer
	//most useful when vertices are shared among neighboring triangles
	//buffer is created on the GPU which is where the data needs to
	{
		//describe the buffer
		D3D11_BUFFER_DESC ibd = {};
		ibd.Usage = D3D11_USAGE_IMMUTABLE;					//doesn't change
		ibd.ByteWidth = sizeof(unsigned int) * numIndices;	//number of indices
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;			//tells Direct3D this is an index buffer
		ibd.CPUAccessFlags = 0;								//cannot access the data from C++
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		//specify the initial data for this buffer
		D3D11_SUBRESOURCE_DATA initialIndexData = {};
		initialIndexData.pSysMem = indices;		//pointer to system memory

		//create the buffer with the initial data
		device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
	}
}

Mesh::~Mesh()
{
}
