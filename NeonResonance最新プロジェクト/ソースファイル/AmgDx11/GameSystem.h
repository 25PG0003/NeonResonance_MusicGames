// GameSystem.h
#pragma once
#include "FbxMesh.h"
#include "Math/math.h"

extern float DeltaTime;

#define WIDTH 1920
#define HEIGHT 1080


void DrawSimpleQuad(const Vector3& pos, const Vector3& scale);

#if _DEBUG
#define USE_IMGUI	(1)
#endif

