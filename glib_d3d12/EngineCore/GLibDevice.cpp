#include <GLibDevice.h>
#include <GLibLogger.h>
#include <string>

glib::GLibDevice::~GLibDevice()
{
    if (m_Device)
        m_Device.Reset();

    glib::Logger::DebugLog("GLibDevice resources released successfully.");
}

bool glib::GLibDevice::Initialize(const D3D_FEATURE_LEVEL& level)
{
    m_Level = level;
    m_Hr = D3D12CreateDevice(nullptr, m_Level, IID_PPV_ARGS(m_Device.GetAddressOf()));
    if (SUCCEEDED(m_Hr))
    {
        glib::Logger::DebugLog("GLibDevice initialized successfully.");
        return true;
    }
    else
    {
        glib::Logger::ErrorLog("Failed to initialize GLibDevice. HRESULT: " + std::to_string(m_Hr));
        return false;
    }
}
