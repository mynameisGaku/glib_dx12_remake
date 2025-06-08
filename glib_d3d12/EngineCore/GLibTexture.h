#pragma once
#include <d3d12.h>
#include <string>
#include <Windows.h>
#include <GLibComPtr.h>

namespace glib
{
    class GLibDevice;
    class GLibDescriptorPool;

    class GLibTexture
    {
    public:
        GLibTexture();
        ~GLibTexture();
        bool Initialize(GLibDevice* device, GLibDescriptorPool* pPool, const std::string& filepath);
        ID3D12Resource* Get();

    private:
        ComPtr<ID3D12Resource> m_TextureBuf = nullptr;
        HRESULT m_Hr{};
    };
}