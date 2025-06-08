#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <Windows.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibDevice.h>

using namespace DirectX;

namespace glib
{
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 texcoord;
    };


    class GLibVertexBuffer
    {
    public:

        GLibVertexBuffer() = default;
        ~GLibVertexBuffer();

        // ���_�o�b�t�@�̏�����
        bool Initialize(GLibDevice* device, Vertex* vertices, UINT vertexCount, UINT stride);

        // ���_�o�b�t�@�r���[���擾
        D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() { return m_VertexBufferView; }

        // ���_�����擾
        UINT GetVertexCount() const { return m_VertexCount; }

    private:
        ComPtr<ID3D12Resource>      m_VertexBuffer = nullptr; // ���_�o�b�t�@���\�[�X
        D3D12_VERTEX_BUFFER_VIEW    m_VertexBufferView = {};  // ���_�o�b�t�@�r���[
        UINT                        m_VertexCount = 0;        // ���_��
        UINT                        m_Stride = 0;             // ���_�̃X�g���C�h�i1���_������̃o�C�g���j
        HRESULT                     m_Hr{};
    };
}