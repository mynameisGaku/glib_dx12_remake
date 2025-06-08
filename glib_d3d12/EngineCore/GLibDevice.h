#pragma once
#include <GLibComPtr.h>
#include <d3d12.h>
#include <d3dcommon.h>
#include <GLibLogger.h>
#include <Windows.h>

namespace glib
{
    class GLibDevice
    {
    public:
        GLibDevice() {}
        ~GLibDevice();

        bool Initialize(const D3D_FEATURE_LEVEL& level);

        ID3D12Device* Get() const { return m_Device.Get(); }

        D3D_FEATURE_LEVEL GetLevel() const { return m_Level; }

    private:

        ComPtr<ID3D12Device> m_Device = nullptr;
        D3D_FEATURE_LEVEL m_Level{};

        HRESULT m_Hr = {};
    };
}