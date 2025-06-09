#include "GLibDescriptorHeap.h"
#include <GLibLogger.h>
#include <GLibDevice.h>
#include <GLibDescriptorPool.h>

glib::GLibDescriptorHeap::GLibDescriptorHeap()
    : m_Index(0)
{
}

glib::GLibDescriptorHeap::~GLibDescriptorHeap()
{
    Logger::DebugLog("Descriptor heap resources released successfully.");
}

bool glib::GLibDescriptorHeap::Initialize(GLibDevice* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
    m_pDevice = device;
    m_Desc = desc;
    m_Hr = device->Get()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescriptorHeap.GetAddressOf()));
    if (FAILED(m_Hr))
    {
        Logger::ErrorLog("Failed to create descriptor heap: " + std::to_string(m_Hr));
        return false;
    }
    Logger::DebugLog("Descriptor heap created successfully.");
    return true;
}

bool glib::GLibDescriptorHeap::Initialize(GLibDescriptorPool* pPool, ID3D12DescriptorHeap* descriptorHeap)
{
    return Initialize(nullptr, pPool, descriptorHeap);
}

bool glib::GLibDescriptorHeap::Initialize(GLibDevice* device, GLibDescriptorPool* pPool, ID3D12DescriptorHeap* descriptorHeap)
{
    if (!descriptorHeap) return false;
    m_Desc = descriptorHeap->GetDesc();
    m_DescriptorHeap = descriptorHeap;
    m_pDescriptorPool = pPool;
    m_pDevice = device;
    m_Hr = S_OK; // ¬Œ÷‚Æ‚Ý‚È‚·
    return true;
}

ID3D12DescriptorHeap* glib::GLibDescriptorHeap::Get() const
{
    return m_DescriptorHeap.Get();
}

UINT glib::GLibDescriptorHeap::GetIncrementSize() const
{
    return m_Index * m_pDevice->Get()->GetDescriptorHandleIncrementSize(m_Desc.Type);
}

UINT glib::GLibDescriptorHeap::GetIncrementSizeIndex(UINT idx) const
{
    return idx * m_pDevice->Get()->GetDescriptorHandleIncrementSize(m_Desc.Type);
}

void glib::GLibDescriptorHeap::Release()
{
    m_pDescriptorPool->Free(m_DescriptorHeap.Get());
    m_DescriptorHeap.Reset();
}

UINT glib::GLibDescriptorHeap::GetIndex() const
{
    return m_Index;
}
