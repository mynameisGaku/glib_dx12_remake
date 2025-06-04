#include "GLibDescriptorHeap.h"

glib::GLibDescriptorHeap::GLibDescriptorHeap()
{
}

glib::GLibDescriptorHeap::~GLibDescriptorHeap()
{
    if (m_DescriptorHeap)
    {
        m_pDescriptorPool->Free(m_DescriptorHeap.Get());
        m_DescriptorHeap.Reset();
    }
    Logger::DebugLog("Descriptor heap resources released successfully.");
}

bool glib::GLibDescriptorHeap::Initialize(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
    m_Hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescriptorHeap.GetAddressOf()));
    if (FAILED(m_Hr))
    {
        Logger::ErrorLog("Failed to create descriptor heap: " + std::to_string(m_Hr));
        return false;
    }
    Logger::DebugLog("Descriptor heap created successfully.");
    return true;
}