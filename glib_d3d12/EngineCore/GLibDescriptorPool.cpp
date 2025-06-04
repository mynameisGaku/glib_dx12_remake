#include "GLibDescriptorPool.h"

glib::GLibDescriptorPool::~GLibDescriptorPool()
{
    AllFree();
    m_pDevice = nullptr;
    m_DescriptorHeaps.clear();
    Logger::DebugLog("GLibDescriptorPool resources released successfully.");
}

bool glib::GLibDescriptorPool::Initialize(ID3D12Device* device)
{
    m_pDevice = device;
    if (!m_pDevice)
    {
        Logger::ErrorLog("Failed to initialize GLibDescriptorPool: Device is null.");
        return false;
    }
    m_DescriptorSize = 0;
    return true;
}

ID3D12DescriptorHeap* glib::GLibDescriptorPool::Get(const std::string& name) const
{
    auto it = m_DescriptorHeaps.find(name);
    if (it != m_DescriptorHeaps.end())
    {
        return it->second.Get();
    }
    Logger::ErrorLog("Descriptor heap with name '" + name + "' not found.");
    return nullptr;
}

ID3D12DescriptorHeap* glib::GLibDescriptorPool::Allocate(const std::string& name, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
    if (m_DescriptorHeaps.find(name) != m_DescriptorHeaps.end())
    {
        Logger::ErrorLog("Descriptor heap with name '" + name + "' already exists.");
        return nullptr;
    }
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    m_Hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(descriptorHeap.GetAddressOf()));
    if (FAILED(m_Hr))
    {
        Logger::ErrorLog("Failed to create descriptor heap: " + std::to_string(m_Hr));
        return nullptr;
    }
    m_DescriptorHeaps[name] = descriptorHeap;
    Logger::DebugLog("Descriptor heap '" + name + "' created successfully.");
    return m_DescriptorHeaps[name].Get();
}

void glib::GLibDescriptorPool::Free(const std::string& name)
{
    auto it = m_DescriptorHeaps.find(name);
    if (it != m_DescriptorHeaps.end())
    {
        m_DescriptorHeaps.erase(it);
        Logger::DebugLog("Descriptor heap '" + name + "' freed successfully.");
    }
    else
    {
        Logger::ErrorLog("Descriptor heap with name '" + name + "' not found.");
    }
}

void glib::GLibDescriptorPool::Free(ID3D12DescriptorHeap* descriptorHeap)
{
    if (!descriptorHeap) return;
    for (auto it = m_DescriptorHeaps.begin(); it != m_DescriptorHeaps.end(); ++it)
    {
        if (it->second.Get() == descriptorHeap)
        {
            m_DescriptorHeaps.erase(it);
            Logger::DebugLog("Descriptor heap freed successfully.");
            return;
        }
    }
    Logger::ErrorLog("Descriptor heap not found in pool.");
}

void glib::GLibDescriptorPool::AllFree()
{
    for (auto& pair : m_DescriptorHeaps)
    {
        pair.second.Reset();
    }
    m_DescriptorHeaps.clear();
    Logger::DebugLog("All descriptor heaps freed successfully.");
}
