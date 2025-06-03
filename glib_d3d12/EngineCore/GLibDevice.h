#pragma once
#include <GLibComPtr.h>
#include <d3d12.h>
#include <d3dcommon.h>
#include "GLibLogger.h"

namespace glib
{
    class GLibDevice
    {
    private:
        GLibDevice() = default;
        ~GLibDevice() = default;

        GLibDevice(const GLibDevice&) = delete;
        GLibDevice& operator=(const GLibDevice&) = delete;

        GLibDevice(GLibDevice&&) noexcept = default;
        GLibDevice& operator=(GLibDevice&&) noexcept = default;
    public:
        static GLibDevice& GetInstance()
        {
            if (not m_Instance)
            {
                m_Instance = new GLibDevice();
            }
            return *m_Instance;
        }

        static void Release()
        {
            if (m_Instance)
            {
                delete m_Instance;
            }
            m_Instance = nullptr;

            glib::Logger::DebugLog("GLibDevice released successfully.");
        }

        bool Initialize(const D3D_FEATURE_LEVEL& level);

        ID3D12Device* Get() const { return m_Device.Get(); }

    private:
        static GLibDevice* m_Instance;

        ComPtr<ID3D12Device> m_Device = nullptr;

        HRESULT m_Hr = {};
    };
}