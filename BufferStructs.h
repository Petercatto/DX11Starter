#pragma once

#include <DirectXMath.h>

//vertext shader struct definition
struct VertexShaderData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};