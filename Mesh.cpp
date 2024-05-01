#include "Mesh.h"
#include <fstream>
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

//helper method to create the buffers
void Mesh::CreateBuffers(Vertex* vertices, int numVertices, UINT* indices, int numIndices)
{
	//create a vertex buffer
	//holds the vertex data of triangles for a single object
	//buffer is created on the GPU which is where the data needs to go
	{
		//describe the buffer we want direct3D to make on the GPU
		//this variable is created on the stack since we only need it once
		//after the buffer is created this description variable is unnecessary
		D3D11_BUFFER_DESC vbd = {};
		vbd.Usage = D3D11_USAGE_DYNAMIC;				//doesn't change
		vbd.ByteWidth = sizeof(Vertex) * numVertices;	//number of vertices
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;		//tells Direct3D this is a vertex buffer
		vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//cannot access the data from C++
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

void Mesh::Draw()
{
	//draw geometry
	//steps are repeated for each object
    UINT stride = sizeof(Vertex);
    UINT offset = 0;

	//set buffers in the input assembler (IA) stage
    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the primitive topology to triangle list
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //tell direct3D to draw
	//begins the rendering pipeline on the GPU
	//uses current direct3D resources such as shaders, buffers, etc
	//DrawIndexed() uses the index buffer to look up corresponding vertices in the vertex buffer
    context->DrawIndexed(
        indexCount,	//number of indices to use
        0,			//offset to the first index
        0);			//offset to add to each index when looking up vertices
}

// --------------------------------------------------------
// Author: Chris Cascioli
// Purpose: Calculates the tangents of the vertices in a mesh
// --------------------------------------------------------
void Mesh::CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices)
{
	// Reset tangents
	for (int i = 0; i < numVerts; i++)
	{
		verts[i].Tangent = XMFLOAT3(0, 0, 0);
	}

	// Calculate tangents one whole triangle at a time
	for (int i = 0; i < numIndices;)
	{
		// Grab indices and vertices of first triangle
		unsigned int i1 = indices[i++];
		unsigned int i2 = indices[i++];
		unsigned int i3 = indices[i++];
		Vertex* v1 = &verts[i1];
		Vertex* v2 = &verts[i2];
		Vertex* v3 = &verts[i3];

		// Calculate vectors relative to triangle positions
		float x1 = v2->Position.x - v1->Position.x;
		float y1 = v2->Position.y - v1->Position.y;
		float z1 = v2->Position.z - v1->Position.z;

		float x2 = v3->Position.x - v1->Position.x;
		float y2 = v3->Position.y - v1->Position.y;
		float z2 = v3->Position.z - v1->Position.z;

		// Do the same for vectors relative to triangle uv's
		float s1 = v2->UV.x - v1->UV.x;
		float t1 = v2->UV.y - v1->UV.y;

		float s2 = v3->UV.x - v1->UV.x;
		float t2 = v3->UV.y - v1->UV.y;

		// Create vectors for tangent calculation
		float r = 1.0f / (s1 * t2 - s2 * t1);

		float tx = (t2 * x1 - t1 * x2) * r;
		float ty = (t2 * y1 - t1 * y2) * r;
		float tz = (t2 * z1 - t1 * z2) * r;

		// Adjust tangents of each vert of the triangle
		v1->Tangent.x += tx;
		v1->Tangent.y += ty;
		v1->Tangent.z += tz;

		v2->Tangent.x += tx;
		v2->Tangent.y += ty;
		v2->Tangent.z += tz;

		v3->Tangent.x += tx;
		v3->Tangent.y += ty;
		v3->Tangent.z += tz;
	}

	// Ensure all of the tangents are orthogonal to the normals
	for (int i = 0; i < numVerts; i++)
	{
		// Grab the two vectors
		XMVECTOR normal = XMLoadFloat3(&verts[i].Normal);
		XMVECTOR tangent = XMLoadFloat3(&verts[i].Tangent);

		// Use Gram-Schmidt orthonormalize to ensure
		// the normal and tangent are exactly 90 degrees apart
		tangent = XMVector3Normalize(
			tangent - normal * XMVector3Dot(normal, tangent));

		// Store the tangent
		XMStoreFloat3(&verts[i].Tangent, tangent);
	}
}

void Mesh::UpdateSnow(float sphereX, float sphereZ, float sphereRadius)
{
	int numVerts = vertexCount;
	int index = rand() % numVerts;
	float offset = static_cast<float>(rand()) / RAND_MAX * 0.1f - 0.005f; //adjust the range

	//update the selected vertex
	vertices[index].Position.y += offset;

	//check if the selected vertex is within the sphere's radius
	DirectX::XMVECTOR spherePos = DirectX::XMVectorSet(sphereX, 0.0f, sphereZ, 0.0f);
	for (int i = 0; i < numVerts; i++)
	{
		DirectX::XMVECTOR vertexPos = DirectX::XMLoadFloat3(&vertices[i].Position);
		float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(vertexPos, spherePos)));
		if (distance < sphereRadius)
		{
			//reset the position of the vertex to 0
			vertices[i].Position.y = 0.0f;
		}
	}

	//update the neighboring vertices
	const float neighborScale = 0.5f; //adjust the influence of neighbors
	int neighborCount = 0;
	for (int i = 0; i < numVerts; i++)
	{
		if (i == index)
			continue;

		DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&vertices[i].Position), DirectX::XMLoadFloat3(&vertices[index].Position));
		float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));

		if (distance < 1.0f) //adjust the radius of influence
		{
			vertices[i].Position.y += offset * neighborScale;
			neighborCount++;
		}
	}

	//map buffers to GPU
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, vertices, sizeof(Vertex) * numVerts);

	context->Unmap(vertexBuffer.Get(), 0);
}

Mesh::Mesh(Microsoft::WRL::ComPtr<ID3D11DeviceContext> c, 
	Microsoft::WRL::ComPtr<ID3D11Device> d, 
	Vertex* verts, int numVertices, 
	UINT* indices, int numIndices)
{
	//initialize indexCount
	indexCount = numIndices;

	//initialize device context
	context = c;
	device = d;

	CalculateTangents(verts, numVertices, indices, numIndices);

	CreateBuffers(verts, numVertices, indices, numIndices);

	//update vertex info
	vertexCount = numVertices;
	vertices = new Vertex[numVertices];

	for (int i = 0; i < numVertices; i++)
	{
		vertices[i] = verts[i];
	}
}

Mesh::Mesh(const wchar_t* fileName, 
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> c, 
	Microsoft::WRL::ComPtr<ID3D11Device> d)
{
	// Author: Chris Cascioli
	// Purpose: Basic .OBJ 3D model loading, supporting positions, uvs and normals

	// File input object
	std::ifstream obj(fileName);

	// Check for successful open
	if (!obj.is_open())
		return;

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;	// Positions from the file
	std::vector<XMFLOAT3> normals;		// Normals from the file
	std::vector<XMFLOAT2> uvs;		// UVs from the file
	std::vector<Vertex> verts;		// Verts we're assembling
	std::vector<UINT> indices;		// Indices of these verts
	int vertCounter = 0;			// Count of vertices
	int indexCounter = 0;			// Count of indices
	char chars[100];			// String for line reading

	// Still have data left?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			// NOTE: This assumes the given obj file contains
			//  vertex positions, uv coordinates AND normals.
			unsigned int i[12];
			int numbersRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// If we only got the first number, chances are the OBJ
			// file has no UV coordinates.  This isn't great, but we
			// still want to load the model without crashing, so we
			// need to re-read a different pattern (in which we assume
			// there are no UVs denoted for any of the vertices)
			if (numbersRead == 1)
			{
				// Re-read with a different pattern
				numbersRead = sscanf_s(
					chars,
					"f %d//%d %d//%d %d//%d %d//%d",
					&i[0], &i[2],
					&i[3], &i[5],
					&i[6], &i[8],
					&i[9], &i[11]);

				// The following indices are where the UVs should 
				// have been, so give them a valid value
				i[1] = 1;
				i[4] = 1;
				i[7] = 1;
				i[10] = 1;

				// If we have no UVs, create a single UV coordinate
				// that will be used for all vertices
				if (uvs.size() == 0)
					uvs.push_back(XMFLOAT2(0, 0));
			}

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			// The model is most likely in a right-handed space,
			// especially if it came from Maya.  We want to convert
			// to a left-handed space for DirectX.  This means we 
			// need to:
			//  - Invert the Z position
			//  - Invert the normal's Z
			//  - Flip the winding order
			// We also need to flip the UV coordinate since DirectX
			// defines (0,0) as the top left of the texture, and many
			// 3D modeling packages use the bottom left as (0,0)

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Flip Z (LH vs. RH)
			v1.Position.z *= -1.0f;
			v2.Position.z *= -1.0f;
			v3.Position.z *= -1.0f;

			// Flip normal's Z
			v1.Normal.z *= -1.0f;
			v2.Normal.z *= -1.0f;
			v3.Normal.z *= -1.0f;

			// Add the verts to the vector (flipping the winding order)
			verts.push_back(v1);
			verts.push_back(v3);
			verts.push_back(v2);
			vertCounter += 3;

			// Add three more indices
			indices.push_back(indexCounter); indexCounter += 1;
			indices.push_back(indexCounter); indexCounter += 1;
			indices.push_back(indexCounter); indexCounter += 1;

			// Was there a 4th face?
			// - 12 numbers read means 4 faces WITH uv's
			// - 8 numbers read means 4 faces WITHOUT uv's
			if (numbersRead == 12 || numbersRead == 8)
			{
				// Make the last vertex
				Vertex v4;
				v4.Position = positions[i[9] - 1];
				v4.UV = uvs[i[10] - 1];
				v4.Normal = normals[i[11] - 1];

				// Flip the UV, Z pos and normal's Z
				v4.UV.y = 1.0f - v4.UV.y;
				v4.Position.z *= -1.0f;
				v4.Normal.z *= -1.0f;

				// Add a whole triangle (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v4);
				verts.push_back(v3);
				vertCounter += 3;

				// Add three more indices
				indices.push_back(indexCounter); indexCounter += 1;
				indices.push_back(indexCounter); indexCounter += 1;
				indices.push_back(indexCounter); indexCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();

	// - At this point, "verts" is a vector of Vertex structs, and can be used
	//    directly to create a vertex buffer:  &verts[0] is the address of the first vert
	//
	// - The vector "indices" is similar. It's a vector of unsigned ints and
	//    can be used directly for the index buffer: &indices[0] is the address of the first int
	//
	// - "vertCounter" is the number of vertices
	// - "indexCounter" is the number of indices
	// - Yes, these are effectively the same since OBJs do not index entire vertices!  This means
	//    an index buffer isn't doing much for us.  We could try to optimize the mesh ourselves
	//    and detect duplicate vertices, but at that point it would be better to use a more
	//    sophisticated model loading library like TinyOBJLoader or The Open Asset Importer Library

	//initialize indexCount
	indexCount = indexCounter;

	//initialize device context
	context = c;
	device = d;

	CalculateTangents(&verts[0], vertCounter, &indices[0], indexCounter);

	CreateBuffers(&verts[0], vertCounter, &indices[0], indexCounter);
}

Mesh::~Mesh()
{
	delete[] vertices;
}
