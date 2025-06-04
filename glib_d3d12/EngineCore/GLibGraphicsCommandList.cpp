#include "GLibGraphicsCommandList.h"

glib::GLibGraphicsCommandList::~GLibGraphicsCommandList()
{
    if (m_CommandList)
        m_CommandList.Reset();

    Logger::DebugLog("Graphics command list resources released successfully.");
}

void glib::GLibGraphicsCommandList::Initialize(ID3D12Device* device, ID3D12CommandAllocator* commandAllocator, const D3D12_COMMAND_LIST_TYPE& type)
{
    m_Hr = device->CreateCommandList(0, type, commandAllocator, nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
    if (FAILED(m_Hr))
    {
        Logger::FormatErrorLog("Failed to create graphics command list. HRESULT: 0x{:X}", m_Hr);
        return;
    }

    Logger::DebugLog("Graphics command list created successfully.");
    m_CommandList->Close();
}
