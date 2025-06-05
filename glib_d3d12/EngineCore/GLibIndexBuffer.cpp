#include <GLibIndexBuffer.h>

bool GLibIndexBuffer::Initialize(ID3D12Device* device, const void* indexData, size_t size, UINT indexCount)
{
    if (!device || !indexData || size == 0 || indexCount == 0) {
        glib::Logger::ErrorLog("Invalid parameters for index buffer initialization.");
        return false;
    }

    // インデックスバッファのリソースを作成
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = size;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    m_Hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_IndexBuffer)
    );

    if (FAILED(m_Hr)) {
        glib::Logger::ErrorLog("Failed to create index buffer resource.");
        return false;
    }

    // インデックスバッファビューの設定
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.SizeInBytes = static_cast<UINT>(size);
    m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32ビットインデックス

    // データをインデックスバッファにコピー
    void* mappedData;
    m_Hr = m_IndexBuffer->Map(0, nullptr, &mappedData);

    if (FAILED(m_Hr)) {
        glib::Logger::ErrorLog("Failed to map index buffer.");
        return false;
    }

    memcpy(mappedData, indexData, size);
    m_IndexBuffer->Unmap(0, nullptr);

    m_IndexCount = indexCount;

    return true;
}
