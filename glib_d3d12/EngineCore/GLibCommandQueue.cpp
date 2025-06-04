#include "GLibCommandQueue.h"

glib::GLibCommandQueue::~GLibCommandQueue()
{
    if (m_CommandQueue)
        m_CommandQueue.Reset();

    Logger::DebugLog("Command queue resources released successfully.");
}

bool glib::GLibCommandQueue::Initialize(ID3D12Device* device, const D3D12_COMMAND_QUEUE_DESC& desc)
{
    m_Hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
    if (FAILED(m_Hr))
    {
        Logger::FormatErrorLog("Failed to create command queue. HRESULT: 0x{:X}", m_Hr);
        return false;
    }

    Logger::DebugLog("Command queue created successfully.");
    return true;
}
