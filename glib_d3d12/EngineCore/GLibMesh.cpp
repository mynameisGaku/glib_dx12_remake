#include "GLibMesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <codecvt>
#include <cassert>
#include <GLib.h>
#include <GLibLogger.h>


#define GLIB_FMT_FLOAT3 DXGI_FORMAT_R32G32B32_FLOAT
#define GLIB_FMT_FLOAT2 DXGI_FORMAT_R32G32_FLOAT
#define GLIB_APPEND     D3D12_APPEND_ALIGNED_ELEMENT
#define GLIB_IL_VERTEX  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA

const D3D12_INPUT_ELEMENT_DESC GLibMeshVertex::InputElements[] =
{
    {"POSITION",    0, GLIB_FMT_FLOAT3, 0, GLIB_APPEND, GLIB_IL_VERTEX, 0},
    {"NORMAL",      0, GLIB_FMT_FLOAT3, 0, GLIB_APPEND, GLIB_IL_VERTEX, 0},
    {"TEXCOORD",    0, GLIB_FMT_FLOAT2, 0, GLIB_APPEND, GLIB_IL_VERTEX, 0},
    {"TANGENT",     0, GLIB_FMT_FLOAT3, 0, GLIB_APPEND, GLIB_IL_VERTEX, 0},
};

const D3D12_INPUT_LAYOUT_DESC GLibMeshVertex::InputLayout =
{
    GLibMeshVertex::InputElements,
    GLibMeshVertex::InputElementCount
};

static_assert(sizeof(GLibMeshVertex) == 44, "Vertex struct/layout mismatch.");

void Failed(const std::string& message)
{
    glib::Logger::FormatErrorLog("���b�V���̃��[�h�Ɏ��s���܂����B[%s]", message);
}

namespace glib
{
    class GLibMeshLoader
    {
    public:
        GLibMeshLoader();
        ~GLibMeshLoader();

        bool Load(const wchar_t* filename, vector<GLibMesh>& meshes, vector<GLibMaterial>& materials);

    private:
        void parseMesh(GLibMesh& dstMesh, const aiMesh* pSrcMesh);
        void parseMaterial(GLibMaterial& dstMaterial, const aiMaterial* pSrcMaterial);
    };

    GLibMeshLoader::GLibMeshLoader()
    {

    }
    GLibMeshLoader::~GLibMeshLoader()
    {

    }

    bool GLibMeshLoader::Load(const wchar_t* filename, vector<GLibMesh>& meshes, vector<GLibMaterial>& materials)
    {
        if (filename == nullptr)
        {
            Failed("filename��nullptr�ł��B");
            return false;
        }

        auto path = glib::ConvertWideToUTF8(filename);

        Assimp::Importer importer;
        int flag = 0;
        flag |= aiProcess_Triangulate;
        flag |= aiProcess_PreTransformVertices;
        flag |= aiProcess_CalcTangentSpace;
        flag |= aiProcess_GenSmoothNormals;
        flag |= aiProcess_GenUVCoords;
        flag |= aiProcess_RemoveRedundantMaterials;
        flag |= aiProcess_OptimizeMeshes;

        // �ǂݍ���
        auto pScene = importer.ReadFile(path, flag);

        // �`�F�b�N
        if (pScene == nullptr)
        {
            // ���s
            Failed("�V�[���̍쐬�Ɏ��s���܂����B�t�@�C���p�X���m�F���Ă��������B");
            return false;
        }

        // ���b�V���������m��
        meshes.clear();
        meshes.resize(pScene->mNumMeshes);

        // ���b�V���f�[�^�ϊ�
        for (size_t i = 0; i < meshes.size(); i++)
        {
            const auto pMesh = pScene->mMeshes[i];
            parseMesh(meshes[i], pMesh);
        }

        // �}�e���A���������m��
        materials.clear();
        materials.resize(pScene->mNumMaterials);

        // �}�e���A���f�[�^�ϊ�
        for (size_t i = 0; i < materials.size(); i++)
        {
            const auto pMaterial = pScene->mMaterials[i];
            parseMaterial(materials[i], pMaterial);
        }

        // �s�v
        pScene = nullptr;

        glib::Logger::FormatErrorLog("���b�V���̃��[�h�ɐ������܂����B");

        return true;
    }

    void GLibMeshLoader::parseMesh(GLibMesh& dstMesh, const aiMesh* pSrcMesh)
    {
        // �}�e���A���ԍ���ݒ�
        dstMesh.MaterialID = pSrcMesh->mMaterialIndex;

        aiVector3D zero3D(0.0f, 0.0f, 0.0f);

        // ���_�f�[�^�̃������m��
        dstMesh.Vertices.resize(pSrcMesh->mNumVertices);

        for (auto i = 0u; i < pSrcMesh->mNumVertices; i++)
        {
            auto pPosition = &(pSrcMesh->mVertices[i]);
            auto pNormal = &(pSrcMesh->mNormals[i]);
            auto pTexCoord = (pSrcMesh->HasTextureCoords(0)) ? &(pSrcMesh->mTextureCoords[0][i]) : &zero3D;
            auto pTangent = (pSrcMesh->HasTangentsAndBitangents()) ? &(pSrcMesh->mTangents[i]) : &zero3D;

            dstMesh.Vertices[i] = GLibMeshVertex(
                XMFLOAT3(pPosition->x, pPosition->y, pPosition->z),
                XMFLOAT3(pNormal->x, pNormal->y, pNormal->z),
                XMFLOAT2(pTexCoord->x, pTexCoord->y),
                XMFLOAT3(pTangent->x, pTangent->y, pTangent->z)
            );
        }

        // ���_�C���f�b�N�X�̃������m��
        dstMesh.Indices.resize(pSrcMesh->mNumFaces * 3);

        for (auto i = 0u; i < pSrcMesh->mNumFaces; i++)
        {
            const auto& face = pSrcMesh->mFaces[i];

            // aiProcess_Triangluated�t���O�ɂ��O�p�`�|���S��������Ă���̂ŁA���_�͕K��3�ɂȂ�
            if (face.mNumIndices != 3)
            {
                Failed("�s���Ȓ��_�C���f�b�N�X�ł��B");
                return;
            }

            dstMesh.Indices[i * 3 + 0] = face.mIndices[0];
            dstMesh.Indices[i * 3 + 1] = face.mIndices[1];
            dstMesh.Indices[i * 3 + 2] = face.mIndices[2];
        }
    }

    void GLibMeshLoader::parseMaterial(GLibMaterial& dstMaterial, const aiMaterial* pSrcMaterial)
    {
        // �g�U
        {
            aiColor3D color(0.0f, 0.0f, 0.0f);

            if (pSrcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
            {
                dstMaterial.Diffuse.x = color.r;
                dstMaterial.Diffuse.y = color.g;
                dstMaterial.Diffuse.z = color.b;
            }
            else
            {
                dstMaterial.Diffuse.x = 0.5f;
                dstMaterial.Diffuse.y = 0.5f;
                dstMaterial.Diffuse.z = 0.5f;
            }
        }

        // ����
        {
            aiColor3D color(0.0f, 0.0f, 0.0f);

            if (pSrcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
            {
                dstMaterial.Specular.x = color.r;
                dstMaterial.Specular.y = color.g;
                dstMaterial.Specular.z = color.b;
            }
            else
            {
                dstMaterial.Specular.x = 0.5f;
                dstMaterial.Specular.y = 0.5f;
                dstMaterial.Specular.z = 0.5f;
            }
        }

        // ���ʔ��ˋ��x
        {
            auto shininess = 0.0f;
            if (pSrcMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
            {
                dstMaterial.Shininess = shininess;
            }
            else
            {
                dstMaterial.Shininess = 0.0f;
            }
        }

        // �f�B�t���[�Y�}�b�v
        {
            aiString path;
            if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == AI_SUCCESS)
            {
                dstMaterial.DiffuseMap = std::string(path.C_Str());
            }
            else
            {
                dstMaterial.DiffuseMap.clear();
            }
        }
    }
}

bool LoadMesh(const wchar_t* filename, vector<GLibMesh>& meshes, vector<GLibMaterial>& materials)
{
    glib::GLibMeshLoader loader;
    return loader.Load(filename, meshes, materials);
}
