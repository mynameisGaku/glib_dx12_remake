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

        // �p�C�v���C���X�e�[�g�̏�����
        bool Initialize(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);

        // �p�C�v���C���X�e�[�g���擾
        ID3D12PipelineState* Get() const { return m_PipelineState.Get(); }

    private:
        ComPtr<ID3D12PipelineState> m_PipelineState = nullptr;
        HRESULT m_Hr;
    };
}