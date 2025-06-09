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

        float GetTextureWidth() const { return m_TexWidth; }
        float GetTextureHeight() const { return m_TexHeight; }

    private:
        ComPtr<ID3D12Resource> m_TextureBuf = nullptr;
        float m_TexWidth;
        float m_TexHeight;
        HRESULT m_Hr{};
    };
}