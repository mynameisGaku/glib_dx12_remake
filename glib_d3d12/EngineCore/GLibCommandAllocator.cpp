#include "GLibCommandAllocator.h"

/* static instance initialize*/
glib::GLibCommandAllocator* glib::GLibCommandAllocator::m_Instance = nullptr;

bool glib::GLibCommandAllocator::Initialize(ID3D12Device* device, const D3D12_COMMAND_LIST_TYPE& type)
{
    m_Hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf()));
    if (FAILED(m_Hr))
    {
        Logger::FormatErrorLog("Failed to create command allocator. HRESULT: 0x{:X}", m_Hr);
        return false;
    }

    Logger::DebugLog("Command allocator created successfully.");
    return true;
}
