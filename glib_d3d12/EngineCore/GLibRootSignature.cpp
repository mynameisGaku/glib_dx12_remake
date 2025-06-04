#include <GLibRootSignature.h>

glib::GLibRootSignature::~GLibRootSignature()
{
    if (m_Rootsignature)
    {
        m_Rootsignature.Reset();
        glib::Logger::FormatDebugLog("GLibRootSignature resources released.");
    }
    m_Rootsignature = nullptr;
    glib::Logger::FormatDebugLog("GLibRootSignature resources released successfully.");
}

bool glib::GLibRootSignature::Initialize(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& rootSignatureDesc)
{
    ID3DBlob* blob;
    m_Hr = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &blob,
        nullptr
    );
    glib::Logger::FormatDebugLog("Serializing root signature...");

    if (FAILED(m_Hr))
    {
        glib::Logger::FormatErrorLog("Failed to serialize root signature: HRESULT = 0x{:X}", m_Hr);
        return false;
    }

    m_Hr = device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_Rootsignature.GetAddressOf()));
    glib::Logger::FormatDebugLog("Creating root signature...");

    if (FAILED(m_Hr))
    {
        glib::Logger::FormatErrorLog("Failed to create root signature: HRESULT = 0x{:X}", m_Hr);
        blob->Release();
        return false;
    }

    blob->Release();
    glib::Logger::FormatDebugLog("Root signature created successfully.");
}