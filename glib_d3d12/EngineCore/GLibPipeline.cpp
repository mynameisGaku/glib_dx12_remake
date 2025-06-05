#include <GLibPipeline.h>
#include <utility>

glib::GLibPipeline::~GLibPipeline()
{
    // ルートシグネチャ解放
    if (m_RootSignature)
    {
        m_RootSignature.Reset();
        glib::Logger::FormatDebugLog("GLibPipeline root signature released successfully.");
    }

    if (m_PipelineState)
    {
        m_PipelineState.Reset();
        glib::Logger::FormatDebugLog("GLibPipeline resources released successfully.");
    }
    m_PipelineState = nullptr;
    glib::Logger::FormatDebugLog("GLibPipeline destroyed successfully.");
}

bool glib::GLibPipeline::Initialize(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
{
    m_Hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState));
    if (FAILED(m_Hr))
    {
        glib::Logger::FormatErrorLog("Failed to create graphics pipeline state: {%x}", m_Hr);
        return false;
    }
    glib::Logger::FormatDebugLog("Graphics pipeline state created successfully.");

    // get rootsignature from desc
    m_RootSignature = std::move(psoDesc.pRootSignature);

    return true;
}
