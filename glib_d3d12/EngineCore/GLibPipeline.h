#pragma once
#include <d3d12.h>
#include <Windows.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibDevice.h>

namespace glib
{
    class GLibPipeline
    {
    public:

        GLibPipeline() {}
        ~GLibPipeline();

        // パイプラインステートの初期化
        bool Initialize(GLibDevice* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);

        // パイプラインステートを取得
        ID3D12PipelineState* Get() const { return m_PipelineState.Get(); }
        ID3D12RootSignature* GetRootSignature() const { return m_RootSignature.Get(); }

    private:
        ComPtr<ID3D12PipelineState> m_PipelineState = nullptr;
        ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;
        HRESULT m_Hr{};
    };
}