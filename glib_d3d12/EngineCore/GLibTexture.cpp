#include <GLib.h>
#include <GLibTexture.h>
#include <GLibLogger.h>
#include <GLibDescriptorPool.h>
#include <GLibDescriptorHeap.h>
#include <GLibDevice.h>
#include <Vendor/stbi/stb_image.h>
#include <dxgiformat.h>
#include <winerror.h>
#include <combaseapi.h>

glib::GLibTexture::GLibTexture()
{
}

glib::GLibTexture::~GLibTexture()
{
    if (m_TextureBuf)
    {
        m_TextureBuf.Reset();
        glib::Logger::DebugLog("Texture buffer resource released successfully.");
    }
}

bool glib::GLibTexture::Initialize(GLibDevice* device, GLibDescriptorPool* pPool, const std::string& filepath)
{
    unsigned char* pixels = nullptr;
    int width = 0, height = 0, bytePerPixel = 4;
    stbi_set_flip_vertically_on_load(true);
    pixels = stbi_load(filepath.c_str(), &width, &height, nullptr, bytePerPixel);
    if (pixels == nullptr)
    {
        glib::Logger::FormatErrorLog("Texture load failed. file: %s", filepath.c_str());
        return false;
    }

    D3D12_HEAP_PROPERTIES prop = {};
    prop.Type = D3D12_HEAP_TYPE_CUSTOM;
    prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    prop.CreationNodeMask = 0;
    prop.VisibleNodeMask = 0;
    D3D12_RESOURCE_DESC desc = {};
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.SampleDesc.Count = 1; // no antialias
    desc.MipLevels = 1;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    m_Hr = device->Get()->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_TextureBuf));
    if (FAILED(m_Hr))
    {
        glib::Logger::ErrorLog("Failed to create texture buffer resource.");
        return false;
    }
    glib::Logger::DebugLog("Texture buffer resource created successfully.");

    m_Hr = m_TextureBuf->WriteToSubresource(0, nullptr, pixels, width * bytePerPixel, width * height * bytePerPixel);

    stbi_image_free(pixels);

    auto pHeap = pPool->Get(GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    UINT increment = pHeap->GetIncrementSize();
    auto hCbvHeap = pHeap->Get()->GetCPUDescriptorHandleForHeapStart();
    hCbvHeap.ptr += increment;
    pHeap->AddIndex();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_TextureBuf->GetDesc().Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    device->Get()->CreateShaderResourceView(m_TextureBuf.Get(), &srvDesc, hCbvHeap);

    glib::Logger::FormatInfoLog("Texture load successfully. file: %s, byte: %d", filepath.c_str(), bytePerPixel);

    return true;
}

ID3D12Resource* glib::GLibTexture::Get()
{
    return m_TextureBuf.Get();
}
