#pragma once

#include "DXCore.h"
#include <wrl/client.h>
#include <memory>
#include "Material.h"
#include "Camera.h"
#include "Transform.h"
#include "SimpleShader.h"

//struct for particle data
struct Particle
{
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT3 StartPos;
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Velocity;
	float Size;
	float Age;
	float StartRot;
	float EndRot;
	float Rot;
};

//struct to be passed into the shader
struct ParticleVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT4 Color;
};

class Emitter
{
public:
	//constructor
	Emitter(int maxPTC,
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
		std::shared_ptr<Material> mat);

	//descructor
	~Emitter();

	//methods
	void Update(float dt);
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> c, std::shared_ptr<Camera> cam);

	//getters
	Transform& GetTransform();
	std::shared_ptr<Material> GetMaterial();

	//setters
	void SetMaterial(std::shared_ptr<Material> mat);

private:
	//emission data
	int particlesPerSecond;
	float secondsPerParticle;
	float timeSinceEmit;

	DirectX::XMFLOAT3 emitterAcceleration;
	DirectX::XMFLOAT3 startVelocity;

	DirectX::XMFLOAT3 positionVariance;
	DirectX::XMFLOAT3 velocityVariance;
	DirectX::XMFLOAT4 rotationVariance; //min start, max star, min end, max end

	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	float startSize;
	float endSize;

	//particles data
	int livingParticles;
	float lifeTime;

	Particle* particles;
	int maxParticles;
	int firstDeadPTCIndex;
	int firstAlivePTCIndex;

	DirectX::XMFLOAT2 DefaultUVs[4];

	//buffers and data
	ParticleVertex* particleVertices;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	//transform
	Transform transform;
	
	//material
	std::shared_ptr<Material> material;

	//update methods
	void UpdateOneParticle(float dt, int index);
	void SpawnParticles();

	//copy methods
	void CopyParticlesToGPU(Microsoft::WRL::ComPtr<ID3D11DeviceContext> c, std::shared_ptr<Camera> cam);
	void CopyOneParticle(int index, std::shared_ptr<Camera> cam);
	DirectX::XMFLOAT3 CalcParticleVertexPosition(int index, int quadCornerIndex, std::shared_ptr<Camera> cam);
};

