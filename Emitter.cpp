#include "Emitter.h"

using namespace DirectX;

Emitter::Emitter(int maxPTC,
	int PTCPerSecond,
	float lTime,
	float sSize,
	float eSize,
	DirectX::XMFLOAT4 sColor,
	DirectX::XMFLOAT4 eColor,
	DirectX::XMFLOAT3 sVel,
	DirectX::XMFLOAT3 velVariance,
	DirectX::XMFLOAT3 emitterPos,
	DirectX::XMFLOAT3 posVariance,
	DirectX::XMFLOAT4 rotVariance,
	DirectX::XMFLOAT3 accceleration,
	Microsoft::WRL::ComPtr<ID3D11Device> d,
	std::shared_ptr<Material> mat)
{
	//assign all params
	material = mat;

	maxParticles = maxPTC;
	particlesPerSecond = PTCPerSecond;
	secondsPerParticle = 1.0f / PTCPerSecond;

	lifeTime = lTime;
	startSize = sSize;
	endSize = eSize;
	startColor = sColor;
	endColor = eColor;
	startVelocity = sVel;

	velocityVariance = velVariance;
	positionVariance = posVariance;
	rotationVariance = rotVariance;

	transform.SetPosition(emitterPos);
	emitterAcceleration = accceleration;

	timeSinceEmit = 0;
	livingParticles = 0;
	firstAlivePTCIndex = 0;
	firstDeadPTCIndex = 0;

	//make blank particle array
	particles = new Particle[maxPTC];
	ZeroMemory(particles, sizeof(Particle) * maxPTC);

	//set up default uvs
	DefaultUVs[0] = XMFLOAT2(0, 0);
	DefaultUVs[1] = XMFLOAT2(1, 0);
	DefaultUVs[2] = XMFLOAT2(1, 1);
	DefaultUVs[3] = XMFLOAT2(0, 1);

	//create uvs
	particleVertices = new ParticleVertex[4 * maxPTC];
	for (int i = 0; i < maxPTC * 4; i += 4)
	{
		particleVertices[i + 0].UV = DefaultUVs[0];
		particleVertices[i + 1].UV = DefaultUVs[1];
		particleVertices[i + 2].UV = DefaultUVs[2];
		particleVertices[i + 3].UV = DefaultUVs[3];
	}

	//create vertex buffer
	D3D11_BUFFER_DESC vBufferDesc = {};
	vBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vBufferDesc.ByteWidth = sizeof(ParticleVertex) * 4 * maxPTC;
	d->CreateBuffer(&vBufferDesc, 0, vertexBuffer.GetAddressOf());

	//index buffer data
	unsigned int* indices = new unsigned int[maxPTC * 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	//create index buffer
	D3D11_BUFFER_DESC iBufferDesc = {};
	iBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iBufferDesc.CPUAccessFlags = 0;
	iBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	iBufferDesc.ByteWidth = sizeof(unsigned int) * maxParticles * 6;
	d->CreateBuffer(&iBufferDesc, &indexData, indexBuffer.GetAddressOf());

	delete[] indices;
}

Emitter::~Emitter()
{
	delete[] particles;
	delete[] particleVertices;
}

//update all particles
void Emitter::Update(float dt)
{
	//check if the first alive particle is before the first dead so particles are contiguous
	if (firstAlivePTCIndex < firstDeadPTCIndex)
	{
		for (int i = firstAlivePTCIndex; i < firstDeadPTCIndex; i++)
		{
			UpdateOneParticle(dt, i);
		}
	}
	//check if the first alive is after the first dead for particles wrapping around
	else
	{
		//update alive
		for (int i = firstAlivePTCIndex; i < maxParticles; i++)
		{
			UpdateOneParticle(dt, i);
		}

		//update dead
		for (int i = 0; i < firstDeadPTCIndex; i++)
		{
			UpdateOneParticle(dt, i);
		}
	}

	//add time since start
	timeSinceEmit += dt;

	//check if there is time to emit the particles
	while (timeSinceEmit > secondsPerParticle)
	{
		SpawnParticles();
		timeSinceEmit -= secondsPerParticle;
	}
}

//draw emitter
void Emitter::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> c, std::shared_ptr<Camera> cam)
{
	//copy to buffer
	CopyParticlesToGPU(c, cam);

	//set up buffers
	UINT stride = sizeof(ParticleVertex);
	UINT offset = 0;
	c->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	c->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	material->GetPixelShader()->SetShader();
	material->GetVertexShader()->SetShader();

	material->GetVertexShader()->SetMatrix4x4("view", cam->GetView());
	material->GetVertexShader()->SetMatrix4x4("projection", cam->GetProjection());

	material->GetPixelShader()->SetFloat3("colorTint", XMFLOAT3(1, 1, 1));

	material->GetVertexShader()->CopyAllBufferData();
	material->GetPixelShader()->CopyAllBufferData();

	//prepare material
	material->PrepareMaterial();

	//draw the right direction on the buffer
	if (firstAlivePTCIndex < firstDeadPTCIndex)
	{
		c->DrawIndexed(livingParticles * 6, firstAlivePTCIndex * 6, 0);
	}
	else
	{
		//draw alive
		c->DrawIndexed(firstDeadPTCIndex * 6, 0, 0);

		//draw dying
		c->DrawIndexed((maxParticles - firstAlivePTCIndex) * 6, firstAlivePTCIndex * 6, 0);
	}
}

Transform& Emitter::GetTransform()
{
	return transform;
}

std::shared_ptr<Material> Emitter::GetMaterial()
{
	return material;
}

void Emitter::SetMaterial(std::shared_ptr<Material> mat)
{
	material = mat;
}

//update a singular particle
void Emitter::UpdateOneParticle(float dt, int index)
{
	//checks particle age
	if (particles[index].Age >= lifeTime)
	{
		//if its past the lifetime stop updating particle
		return;
	}

	//checks for dead particles
	particles[index].Age += dt;
	if (particles[index].Age >= lifeTime)
	{
		//move along alive and dead indices
		firstAlivePTCIndex++;
		firstAlivePTCIndex %= maxParticles;
		livingParticles--;
		return;
	}

	//calculate age
	float agePercentage = particles[index].Age / lifeTime;

	//interpolate color
	XMStoreFloat4(&particles[index].Color, XMVectorLerp(XMLoadFloat4(&startColor), XMLoadFloat4(&endColor), agePercentage));

	//interpolate rotation
	float startRotation = particles[index].StartRot;
	float endRotation = particles[index].EndRot;
	particles[index].Rot = startRotation + agePercentage * (endRotation - startRotation);

	//interpolate size
	particles[index].Size = startSize + agePercentage * (endSize - startSize);

	//update position
	XMVECTOR startPos = XMLoadFloat3(&particles[index].StartPos);
	XMVECTOR startVel = XMLoadFloat3(&particles[index].Velocity);
	XMVECTOR a = XMLoadFloat3(&emitterAcceleration);
	float t = particles[index].Age;

	//update acceleration
	XMStoreFloat3(
		&particles[index].Pos,
		a * t * t / 2.0f + startVel * t + startPos);
}

//reset the dead particles to cycle them into the alive ones to be spawned
void Emitter::SpawnParticles()
{
	//check if all the particles have been spawned
	if (livingParticles == maxParticles)
	{
		return;
	}

	//reset dead particles
	particles[firstDeadPTCIndex].Age = 0;
	particles[firstDeadPTCIndex].Size = startSize;
	particles[firstDeadPTCIndex].Color = startColor;

	particles[firstDeadPTCIndex].StartPos = transform.GetPosition();
	particles[firstDeadPTCIndex].StartPos.x += (((float)rand() / RAND_MAX) * 2 - 1) * positionVariance.x;
	particles[firstDeadPTCIndex].StartPos.y += (((float)rand() / RAND_MAX) * 2 - 1) * positionVariance.y;
	particles[firstDeadPTCIndex].StartPos.z += (((float)rand() / RAND_MAX) * 2 - 1) * positionVariance.z;

	particles[firstDeadPTCIndex].Pos = particles[firstDeadPTCIndex].StartPos;

	particles[firstDeadPTCIndex].Velocity = startVelocity;
	particles[firstDeadPTCIndex].Velocity.x += (((float)rand() / RAND_MAX) * 2 - 1) * velocityVariance.x;
	particles[firstDeadPTCIndex].Velocity.y += (((float)rand() / RAND_MAX) * 2 - 1) * velocityVariance.y;
	particles[firstDeadPTCIndex].Velocity.z += (((float)rand() / RAND_MAX) * 2 - 1) * velocityVariance.z;

	float rotStartMin = rotationVariance.x;
	float rotStartMax = rotationVariance.y;
	particles[firstDeadPTCIndex].StartRot = ((float)rand() / RAND_MAX) * (rotStartMax - rotStartMin) + rotStartMin;

	float rotEndMin = rotationVariance.z;
	float rotEndMax = rotationVariance.w;
	particles[firstDeadPTCIndex].EndRot = ((float)rand() / RAND_MAX) * (rotEndMax - rotEndMin) + rotEndMin;

	//wrap the particles
	firstDeadPTCIndex++;
	firstDeadPTCIndex %= maxParticles;

	livingParticles++;
}

//update the buffers
void Emitter::CopyParticlesToGPU(Microsoft::WRL::ComPtr<ID3D11DeviceContext> c, std::shared_ptr<Camera> cam)
{
	//check if the first alive particle is before the first dead so particles are contiguous
	if (firstAlivePTCIndex < firstDeadPTCIndex)
	{
		for (int i = firstAlivePTCIndex; i < firstDeadPTCIndex; i++)
		{
			CopyOneParticle(i, cam);
		}
	}
	//check if the first alive is after the first dead for particles wrapping around
	else
	{
		//update alive
		for (int i = firstAlivePTCIndex; i < maxParticles; i++)
			CopyOneParticle(i, cam);

		//update dead
		for (int i = 0; i < firstDeadPTCIndex; i++)
			CopyOneParticle(i, cam);
	}

	//map buffers to gpu
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	c->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, particleVertices, sizeof(ParticleVertex) * 4 * maxParticles);

	c->Unmap(vertexBuffer.Get(), 0);
}

//copy singualr particle to gpu
void Emitter::CopyOneParticle(int index, std::shared_ptr<Camera> cam)
{
	int i = index * 4;

	particleVertices[i + 0].Position = CalcParticleVertexPosition(index, 0, cam);
	particleVertices[i + 1].Position = CalcParticleVertexPosition(index, 1, cam);
	particleVertices[i + 2].Position = CalcParticleVertexPosition(index, 2, cam);
	particleVertices[i + 3].Position = CalcParticleVertexPosition(index, 3, cam);

	particleVertices[i + 0].Color = particles[index].Color;
	particleVertices[i + 1].Color = particles[index].Color;
	particleVertices[i + 2].Color = particles[index].Color;
	particleVertices[i + 3].Color = particles[index].Color;
}

DirectX::XMFLOAT3 Emitter::CalcParticleVertexPosition(int index, int quadCornerIndex, std::shared_ptr<Camera> cam)
{
	//get right vector, up vector and view matrix
	XMFLOAT4X4 view = cam->GetView();
	XMVECTOR rightVec = XMVectorSet(view._11, view._21, view._31, 0);
	XMVECTOR upVec = XMVectorSet(view._12, view._22, view._32, 0);

	//calculate offset in the quad based off the corner
	XMFLOAT2 offset = DefaultUVs[quadCornerIndex];
	offset.x = offset.x * 2 - 1;
	offset.y = (offset.y * -2 + 1);	

	//apply z rotation
	XMVECTOR offsetVec = XMLoadFloat2(&offset);
	XMMATRIX rotMatrix = XMMatrixRotationZ(particles[index].Rot);
	offsetVec = XMVector3Transform(offsetVec, rotMatrix);

	//add to position via the offsets
	XMVECTOR posVec = XMLoadFloat3(&particles[index].Pos);
	posVec += rightVec * XMVectorGetX(offsetVec) * particles[index].Size;
	posVec += upVec * XMVectorGetY(offsetVec) * particles[index].Size;

	//store position
	XMFLOAT3 pos;
	XMStoreFloat3(&pos, posVec);
	return pos;
}
