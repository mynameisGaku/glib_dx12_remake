#include "GLibFence.h"
#include <GLibCommandQueue.h>


glib::GLibFence::~GLibFence()
{
    if (m_Fence)
        m_Fence.Reset();

    if (m_FenceEvent)
    {
        CloseHandle(m_FenceEvent);
        m_FenceEvent = nullptr;
    }

    Logger::DebugLog("Fence resources released successfully.");
}

bool glib::GLibFence::Initialize(GLibDevice* device, GLibCommandQueue* queue, const D3D12_FENCE_FLAGS& flags)
{
    m_pDevice = device;
    m_pCommandQueue = queue;

    m_Hr = m_pDevice->Get()->CreateFence(0, flags, IID_PPV_ARGS(&m_Fence));
    if (FAILED(m_Hr))
    {
        Logger::FormatErrorLog("Failed to create fence");
        return false;
    }

    m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_FenceEvent)
    {
        Logger::FormatErrorLog("Failed to create event handle");
        return false;
    }

    Logger::DebugLog("Fence created successfully.");
    return true;
}

void glib::GLibFence::WaitDrawDone()
{
    UINT64 fvalue = m_FenceValue;
    m_pCommandQueue->Get()->Signal(m_Fence.Get(), fvalue);
    m_FenceValue++;

    if (m_Fence->GetCompletedValue() < fvalue)
    {
        m_Fence->SetEventOnCompletion(fvalue, m_FenceEvent);

        WaitForSingleObject(m_FenceEvent, INFINITE);
    }
}

void glib::GLibFence::Close()
{
    if (m_FenceEvent)
    {
        CloseHandle(m_FenceEvent);
        m_FenceEvent = nullptr;
    }

    if (m_Fence)
    {
        m_Fence.Reset();
    }

    Logger::CriticalLog("Fence resources released successfully.");
}
