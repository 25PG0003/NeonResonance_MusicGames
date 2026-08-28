#pragma once
#include "FbxMesh.h"
#include "Math/math.h"

class JudgeLine
{
public:
    JudgeLine();
    ~JudgeLine();

    void Initialize();
    void Update();
    void Draw();

private:
    FBXMesh* mesh;
    Vector3 position;
    Vector3 scale;
};
