#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibVertexBuffer
    {
    public:

        GLibVertexBuffer() = default;
        ~GLibVertexBuffer() = default;

        // 頂点バッファの初期化
        bool Initialize(ID3D12Device* device, const void* vertexData, UINT vertexCount, UINT stride);

        // 頂点バッファビューを取得
        const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return m_VertexBufferView; }

        // 頂点数を取得
        UINT GetVertexCount() const { return m_VertexCount; }

    private:
        ComPtr<ID3D12Resource>      m_VertexBuffer = nullptr; // 頂点バッファリソース
        D3D12_VERTEX_BUFFER_VIEW    m_VertexBufferView = {};  // 頂点バッファビュー
        UINT                        m_VertexCount = 0;        // 頂点数
        UINT                        m_Stride = 0;             // 頂点のストライド（1頂点あたりのバイト数）
        HRESULT                     m_Hr{};
    };
}