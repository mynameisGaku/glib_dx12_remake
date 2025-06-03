#include <GLibConstantBuffer.h>
#include <GLibDescriptorPool.h>
#include <GLibDevice.h>

bool glib::GLibConstantBuffer::Initialize(ID3D12Device* device, const D3D12_RESOURCE_DESC& desc)
{
    // �萔�o�b�t�@�p��Heap���
    if (glib::GLibDescriptorPool::GetInstance().Get("ConstantBufferHeap") == nullptr)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        // DescriptorHeap����CBV,SRV,UAV�͍��݉\
        // DescriptorHeap�̂ǂ͈̔͂��ǂ̃��W�X�^�Ɋ��蓖�Ă邩�́A���[�g�V�O�l�`���쐬����Range��Parameter�Ō��肷��
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // �V�F�[�_�[����A�N�Z�X����
        m_pConstantHeap = glib::GLibDescriptorPool::GetInstance().Allocate("ConstantBufferHeap", desc);
    }

    // �萔�o�b�t�@�̃��\�[�X���쐬
    {
        // �v���p�e�B�ݒ�
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD; // CPU���珑������
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // ���\�[�X�̐���
        m_Hr= glib::GLibDevice::GetInstance().Get()->CreateCommittedResource(
            &prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_ConstantBuffer.GetAddressOf()));

        if (FAILED(m_Hr))
        {
            glib::Logger::FormatErrorLog("Failed to create constant buffer resource. HRESULT: 0x{:X}", m_Hr);
            return false;
        }
    }

    // �萔�o�b�t�@��Descriptor��Heap�ɐݒ肷��
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};

        desc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress();

        desc.SizeInBytes = 256;

        glib::GLibDevice::GetInstance().Get()->CreateConstantBufferView(&desc, m_pConstantHeap->GetCPUDescriptorHandleForHeapStart());

    }

    // �萔�o�b�t�@���}�b�v���Ă���

    m_ConstantBuffer->Map(0, nullptr, &m_pConstBufferData);
}
