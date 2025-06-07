#include <GLibConstantBuffer.h>
#include <GLibDescriptorPool.h>
#include <GLibDevice.h>

UINT glib::GLibConstantBuffer::m_sCurrentIndex = 0;

glib::GLibConstantBuffer::~GLibConstantBuffer()
{
    if (m_pMappedConstBuf)
    {
        m_pMappedConstBuf = nullptr;
        glib::Logger::DebugLog("Constant buffer unmapped successfully.");
    }
    if (m_ConstBuf)
    {
        m_ConstBuf.Reset();
        glib::Logger::DebugLog("Constant buffer resource released successfully.");
    }
    m_hCbvHeap = {};
    m_pCbvHeap = nullptr;
}

bool glib::GLibConstantBuffer::Initialize(GLibDevice* device, GLibDescriptorPool* pPool, const D3D12_RESOURCE_DESC& desc)
{
    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type = D3D12_HEAP_TYPE_UPLOAD;
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    prop.CreationNodeMask = 1;
    prop.VisibleNodeMask = 1;

    m_Hr = device->Get()->CreateCommittedResource(
        &prop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_ConstBuf)
    );
    if (FAILED(m_Hr))
    {
        glib::Logger::ErrorLog("Failed to create constant buffer resource.");
        return false;
    }
    glib::Logger::DebugLog("Constant buffer resource created successfully.");

    m_Hr = m_ConstBuf->Map(0, nullptr, &m_pMappedConstBuf);
    if (FAILED(m_Hr))
    {
        glib::Logger::ErrorLog("Failed to map constant buffer.");
        return false;
    }
    m_ConstBuf->Unmap(0, nullptr);
    glib::Logger::DebugLog("Constant buffer mapped successfully.");

    auto descHeap = pPool->Get(glib::GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    if (!descHeap)
    {
        glib::Logger::ErrorLog("Descriptor heap for constant buffer not found.");
        return false;
    }
    m_Index = m_sCurrentIndex;
    UINT heapSize = device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_hCbvHeap = descHeap->GetCPUDescriptorHandleForHeapStart();
    m_hCbvHeap.ptr += m_Index * heapSize;

    glib::Logger::DebugLog("Descriptor heap for constant buffer obtained successfully.");

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = m_ConstBuf->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(m_ConstBuf->GetDesc().Width);
    device->Get()->CreateConstantBufferView(&cbvDesc, m_hCbvHeap);

    glib::Logger::DebugLog("Constant buffer view created successfully.");
    m_sCurrentIndex++;
    return true;
}
