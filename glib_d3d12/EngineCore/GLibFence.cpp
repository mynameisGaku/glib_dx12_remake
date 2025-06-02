#include "GLibFence.h"

/* static instance initialize*/
glib::GLibFence* glib::GLibFence::m_Instance = nullptr;

bool glib::GLibFence::Initialize(ID3D12Device* device, const D3D12_FENCE_FLAGS& flags)
{
    m_Hr = device->CreateFence(0, flags, IID_PPV_ARGS(&m_Fence));
    if (FAILED(m_Hr))
    {
        Logger::FormatErrorLog("Failed to create fence");
        return false;
    }

    m_EventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_EventHandle)
    {
        Logger::FormatErrorLog("Failed to create event handle");
        return false;
    }

    Logger::DebugLog("Fence created successfully.");
    return true;
}

UINT64 glib::GLibFence::Signal()
{
    m_FenceValue++;
    m_Hr = m_Fence->Signal(m_FenceValue);
    if (FAILED(m_Hr))
    {
        Logger::FormatErrorLog("Failed to signal fence. HRESULT: 0x{:X}", m_Hr);
        return 0;
    }

    // Reset the event handle
    ResetEvent(m_EventHandle);
    return m_FenceValue;
}

void glib::GLibFence::WaitForSignal(UINT64 fenceValue)
{
    if (m_Fence->GetCompletedValue() < fenceValue)
    {
        m_Hr = m_Fence->SetEventOnCompletion(fenceValue, m_EventHandle);
        if (FAILED(m_Hr))
        {
            Logger::FormatErrorLog("Failed to set event on fence completion. HRESULT: 0x{:X}", m_Hr);
            return;
        }

        // Wait for the event to be signaled
        WaitForSingleObject(m_EventHandle, INFINITE);
    }
}

void glib::GLibFence::Close()
{
    if (m_EventHandle)
    {
        CloseHandle(m_EventHandle);
        m_EventHandle = nullptr;
    }

    if (m_Fence)
    {
        m_Fence.Reset();
    }

    Logger::CriticalLog("Fence resources released successfully.");
}
