#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

using namespace DirectX;
using namespace std;

struct GLibMeshVertex
{
    XMFLOAT3 Position;
    XMFLOAT3 Normal;
    XMFLOAT2 TexCoord;
    XMFLOAT3 Tangent;

    GLibMeshVertex() = default;

    GLibMeshVertex(
        const XMFLOAT3& position,
        const XMFLOAT3& normal,
        const XMFLOAT2& texcoord,
        const XMFLOAT3& tangent
    )
        :
        Position(position),
        Normal(normal),
        TexCoord(texcoord),
        Tangent(tangent)
    {}

    static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
    static const int InputElementCount = 4; // 入力レイアウトの数
    static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct GLibMaterial
{
    XMFLOAT3 Diffuse;
    XMFLOAT3 Specular;
    float ALpha;
    float Shininess;
    std::string DiffuseMap;
};

struct GLibMesh
{
    vector<GLibMeshVertex> Vertices;
    vector<UINT32> Indices;
    UINT32 MaterialID;
};

bool LoadMesh(
    const wchar_t* filename,
    vector<GLibMesh>& meshes,
    vector<GLibMaterial>& materials);