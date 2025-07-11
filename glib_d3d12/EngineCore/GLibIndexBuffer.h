#pragma once
#include <d3d12.h>
#include <Windows.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibDevice.h>

namespace glib
{
    class GLibIndexBuffer
    {
    public:
        GLibIndexBuffer() = default;
        ~GLibIndexBuffer();

        // インデックスバッファの初期化
        bool Initialize(GLibDevice* device, const void* indexData, UINT indexCount, UINT stride);

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
}