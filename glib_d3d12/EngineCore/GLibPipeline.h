#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibPipeline
    {
    public:

        GLibPipeline() = default;
        ~GLibPipeline();

        // パイプラインステートの初期化
        bool Initialize(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);

        // パイプラインステートを取得
        ID3D12PipelineState* Get() const { return m_PipelineState.Get(); }

    private:
        ComPtr<ID3D12PipelineState> m_PipelineState = nullptr;
        HRESULT m_Hr;
    };
}