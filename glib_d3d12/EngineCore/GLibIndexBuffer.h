#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

class GLibIndexBuffer
{
public:
    GLibIndexBuffer() = default;
    ~GLibIndexBuffer() = default;

    // �C���f�b�N�X�o�b�t�@�̏�����
    bool Initialize(ID3D12Device* device, const void* indexData, size_t size, UINT indexCount);

    // �C���f�b�N�X�o�b�t�@�r���[���擾
    D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() { return m_IndexBufferView; }

    // �C���f�b�N�X�����擾
    UINT GetIndexCount() const { return m_IndexCount; }
private:
    ComPtr<ID3D12Resource>      m_IndexBuffer = nullptr; // �C���f�b�N�X�o�b�t�@���\�[�X
    D3D12_INDEX_BUFFER_VIEW     m_IndexBufferView = {};  // �C���f�b�N�X�o�b�t�@�r���[
    UINT                        m_IndexCount = 0;        // �C���f�b�N�X��
    HRESULT                     m_Hr{};
};