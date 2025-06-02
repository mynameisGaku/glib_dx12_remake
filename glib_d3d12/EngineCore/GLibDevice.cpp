#include <GLibDevice.h>
#include <GLibLogger.h>
#include <string>

/* static instance initialize*/
glib::GLibDevice* glib::GLibDevice::m_Instance = nullptr;

bool glib::GLibDevice::Initialize(const D3D_FEATURE_LEVEL& level)
{
    m_Hr = D3D12CreateDevice(nullptr, level, IID_PPV_ARGS(m_Device.GetAddressOf()));
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
