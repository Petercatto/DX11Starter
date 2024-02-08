#pragma once

#include <DirectXMath.h>

//vertext shader struct definition
struct VertexShaderData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT3 offset;
};