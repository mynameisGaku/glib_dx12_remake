#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibVertexBuffer
    {
    public:

        GLibVertexBuffer() = default;
        ~GLibVertexBuffer() = default;

        // ���_�o�b�t�@�̏�����
        bool Initialize(ID3D12Device* device, const void* vertexData, UINT vertexCount, UINT stride);

        // ���_�o�b�t�@�r���[���擾
        const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return m_VertexBufferView; }

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