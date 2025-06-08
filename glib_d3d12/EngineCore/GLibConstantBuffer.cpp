#include <GLibConstantBuffer.h>
#include <GLibDescriptorPool.h>
#include <GLibDescriptorHeap.h>
#include <GLibDevice.h>
#include <GLib.h>
#include <Windows.h>
#include <winerror.h>
#include <combaseapi.h>

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

    GLibDescriptorHeap* descHeap = pPool->Get(glib::GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    if (!descHeap)
    {
        glib::Logger::ErrorLog("Descriptor heap for constant buffer not found.");
        return false;
    }
    UINT increment = descHeap->GetIncrementSize();
    m_hCbvHeap = descHeap->Get()->GetCPUDescriptorHandleForHeapStart();
    m_hCbvHeap.ptr += increment;
    descHeap->AddIndex();

    glib::Logger::DebugLog("Descriptor heap for constant buffer obtained successfully.");

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = m_ConstBuf->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(m_ConstBuf->GetDesc().Width);
    device->Get()->CreateConstantBufferView(&cbvDesc, m_hCbvHeap);

    glib::Logger::DebugLog("Constant buffer view created successfully.");
    return true;
}

ID3D12Resource* glib::GLibConstantBuffer::GetResource() const
{
    return m_ConstBuf.Get();
}

ID3D12DescriptorHeap* glib::GLibConstantBuffer::GetDescriptorHeap() const
{
    return m_pCbvHeap;
}

D3D12_CPU_DESCRIPTOR_HANDLE glib::GLibConstantBuffer::GetDescriptorHandle() const
{
    return m_hCbvHeap;
}