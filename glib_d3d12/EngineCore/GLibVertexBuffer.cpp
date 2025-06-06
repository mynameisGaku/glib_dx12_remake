#include <GLibVertexBuffer.h>

glib::GLibVertexBuffer::~GLibVertexBuffer()
{
    if (m_VertexBuffer)
    {
        m_VertexBuffer.Reset();
        glib::Logger::DebugLog("Vertex buffer resources released successfully.");
    }
    else
    {
        glib::Logger::DebugLog("No vertex buffer resources to release.");
    }
}

bool glib::GLibVertexBuffer::Initialize(ID3D12Device* device, const void* vertexData, UINT vertexCount, UINT stride)
{
    if (!device || !vertexData || vertexCount == 0 || stride == 0)
    {
        glib::Logger::ErrorLog("Invalid parameters for vertex buffer initialization.");
        return false;
    }

    m_VertexCount = vertexCount;
    m_Stride = stride;

    size_t bufferSize = vertexCount * stride;

    // 頂点バッファのリソースを作成
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = 1;
    heapProperties.VisibleNodeMask = 1;

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
        IID_PPV_ARGS(&m_VertexBuffer)
    );

    if (FAILED(m_Hr))
    {
        glib::Logger::ErrorLog("Failed to create vertex buffer resource.");
        return false;
    }

    // 頂点データをアップロード
    UINT8* mappedData;
    m_Hr = m_VertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));

    if (FAILED(m_Hr))
    {
        glib::Logger::ErrorLog("Failed to map vertex buffer.");
        return false;
    }

    // 頂点データをコピー
    memcpy(mappedData, vertexData, bufferSize);

    m_VertexBuffer->Unmap(0, nullptr);

    // 頂点バッファビューを設定
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
    m_VertexBufferView.StrideInBytes = stride;

    return true;
}

