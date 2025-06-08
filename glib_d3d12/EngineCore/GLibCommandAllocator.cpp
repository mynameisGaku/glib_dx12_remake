#include "GLibCommandAllocator.h"

glib::GLibCommandAllocator::~GLibCommandAllocator()
{
    if (m_CommandAllocator)
        m_CommandAllocator.Reset();

    Logger::DebugLog("Command allocator resources released successfully.");
}

bool glib::GLibCommandAllocator::Initialize(GLibDevice* device, const D3D12_COMMAND_LIST_TYPE& type)
{
    m_Hr = device->Get()->CreateCommandAllocator(type, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf()));
    if (FAILED(m_Hr))
    {
        Logger::FormatErrorLog("Failed to create command allocator. HRESULT: 0x{:X}", m_Hr);
        return false;
    }

    Logger::DebugLog("Command allocator created successfully.");
    return true;
}
