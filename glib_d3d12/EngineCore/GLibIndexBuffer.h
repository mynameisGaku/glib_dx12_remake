#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

class GLibIndexBuffer
{
public:
    GLibIndexBuffer() = default;
    ~GLibIndexBuffer() = default;

    // インデックスバッファの初期化
    bool Initialize(ID3D12Device* device, const void* indexData, size_t size, UINT indexCount);

    // インデックスバッファビューを取得
    D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() { return m_IndexBufferView; }

    // インデックス数を取得
    UINT GetIndexCount() const { return m_IndexCount; }
private:
    ComPtr<ID3D12Resource>      m_IndexBuffer = nullptr; // インデックスバッファリソース
    D3D12_INDEX_BUFFER_VIEW     m_IndexBufferView = {};  // インデックスバッファビュー
    UINT                        m_IndexCount = 0;        // インデックス数
    HRESULT                     m_Hr{};
};