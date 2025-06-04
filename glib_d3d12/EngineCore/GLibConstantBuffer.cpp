#include <GLibConstantBuffer.h>
#include <GLibDescriptorPool.h>
#include <GLibDevice.h>

glib::GLibConstantBuffer::~GLibConstantBuffer()
{
    if (m_ConstantBuffer)
        m_ConstantBuffer.Reset();

    if (m_pConstantHeap)
        m_pConstantHeap->Release();

    if (m_pConstBufferData)
        m_ConstantBuffer->Unmap(0, nullptr);

    m_pConstBufferData = nullptr;
    m_pDevice = nullptr;
    m_pDescriptorPool = nullptr;
    glib::Logger::DebugLog("Constant buffer resources released successfully.");
}

bool glib::GLibConstantBuffer::Initialize(GLibDevice* device, const D3D12_RESOURCE_DESC& desc)
{
    m_pDevice = device;



    // 定数バッファ用のHeap作る
    if (m_pDescriptorPool->Get("ConstantBufferHeap") == nullptr)
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = 1;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        // DescriptorHeap内にCBV,SRV,UAVは混在可能
        // DescriptorHeapのどの範囲をどのレジスタに割り当てるかは、ルートシグネチャ作成時のRangeとParameterで決定する
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // シェーダーからアクセスする
        m_pConstantHeap = m_pDescriptorPool->Allocate("ConstantBufferHeap", desc);
    }

    // 定数バッファのリソースを作成
    {
        // プロパティ設定
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD; // CPUから書き込む
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // リソースの生成
        m_Hr = m_pDevice->Get()->CreateCommittedResource(
            &prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_ConstantBuffer.GetAddressOf()));

        if (FAILED(m_Hr))
        {
            glib::Logger::FormatErrorLog("Failed to create constant buffer resource. HRESULT: 0x{:X}", m_Hr);
            return false;
        }
    }

    // 定数バッファのDescriptorをHeapに設定する
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};

        desc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress();

        desc.SizeInBytes = 256;

        m_pDevice->Get()->CreateConstantBufferView(&desc, m_pConstantHeap->GetCPUDescriptorHandleForHeapStart());

    }

    // 定数バッファをマップしておく

    m_ConstantBuffer->Map(0, nullptr, &m_pConstBufferData);
}
