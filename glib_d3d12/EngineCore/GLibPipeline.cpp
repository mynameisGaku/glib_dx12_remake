#include <GLibPipeline.h>

glib::GLibPipeline::~GLibPipeline()
{
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
    return true;
}
