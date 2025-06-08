#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <Windows.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibDevice.h>

using namespace DirectX;

namespace glib
{
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT2 texcoord;
    };


    class GLibVertexBuffer
    {
    public:

        GLibVertexBuffer() = default;
        ~GLibVertexBuffer();

        // 頂点バッファの初期化
        bool Initialize(GLibDevice* device, Vertex* vertices, UINT vertexCount, UINT stride);

        // 頂点バッファビューを取得
        D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() { return m_VertexBufferView; }

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