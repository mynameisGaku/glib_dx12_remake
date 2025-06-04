#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibDevice.h>
#include <GLibDescriptorPool.h>

namespace glib
{
    class GLibConstantBuffer
    {
    public:

        GLibConstantBuffer() = default;
        ~GLibConstantBuffer();
        bool Initialize(GLibDevice* device, const D3D12_RESOURCE_DESC& desc);

        ID3D12Resource* Get() const { return m_ConstantBuffer.Get(); }

    private:
        GLibDevice*             m_pDevice = nullptr; // �f�o�C�X�ւ̃|�C���^
        GLibDescriptorPool*     m_pDescriptorPool = nullptr; // �f�B�X�N���v�^�v�[���ւ̃|�C���^
        ID3D12DescriptorHeap*   m_pConstantHeap = nullptr;
        ComPtr<ID3D12Resource>  m_ConstantBuffer = nullptr;
        void*                   m_pConstBufferData = nullptr; // �}�b�v���ꂽ�萔�o�b�t�@�̃|�C���^
        HRESULT                 m_Hr = {};
    };
}