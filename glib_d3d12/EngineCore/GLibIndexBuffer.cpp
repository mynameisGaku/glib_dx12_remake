#include <GLibIndexBuffer.h>

glib::GLibIndexBuffer::~GLibIndexBuffer()
{
    if (m_IndexBuffer)
    {
        m_IndexBuffer.Reset();
        glib::Logger::DebugLog("Index buffer resources released successfully.");
    }
    else
    {
        glib::Logger::DebugLog("No index buffer resources to release.");
    }
}

bool glib::GLibIndexBuffer::Initialize(ID3D12Device* device, const void* indexData, UINT indexCount, UINT stride)
{
    if (!device || !indexData) 
    {
        glib::Logger::ErrorLog("Invalid parameters for index buffer initialization.");
        return false;
    }

    size_t bufferSize = indexCount * stride;

    // インデックスバッファのリソースを作成
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.VisibleNodeMask = 1;
    heapProperties.CreationNodeMask = 1;
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = bufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    m_Hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_IndexBuffer)
    );

    if (FAILED(m_Hr))
    {
        glib::Logger::ErrorLog("Failed to create index buffer resource.");
        return false;
    }

    // データをインデックスバッファにコピー
    UINT8* mappedData;
    m_Hr = m_IndexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));

    if (FAILED(m_Hr)) 
    {
        glib::Logger::ErrorLog("Failed to map index buffer.");
        return false;
    }

    memcpy(mappedData, indexData, bufferSize);
    m_IndexBuffer->Unmap(0, nullptr);

    m_IndexCount = indexCount;

    // インデックスバッファビューの設定
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;


    return true;
}
