#pragma once
#include <GLibComPtr.h>
#include <d3d12.h>
#include <d3dcommon.h>
#include "GLibLogger.h"

namespace glib
{
    class GLibDevice
    {
    public:
        GLibDevice() {}
        ~GLibDevice();

        bool Initialize(const D3D_FEATURE_LEVEL& level);

        ID3D12Device* Get() const { return m_Device.Get(); }

    private:

        ComPtr<ID3D12Device> m_Device = nullptr;

        HRESULT m_Hr = {};
    };
}