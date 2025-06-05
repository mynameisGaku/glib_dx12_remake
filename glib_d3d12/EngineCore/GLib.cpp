#include <GLib.h>
#include <GLibDevice.h>
#include <GLibDebug.h>
#include <GLibCommandAllocator.h>
#include <GLibCommandQueue.h>
#include <GLibGraphicsCommandList.h>
#include <GLibFence.h>
#include <GLibStringUtil.h>
#include <GLibDescriptorPool.h>
#include <GLibSwapChain.h>
#include <GLibTime.h>
#include <GLibPipeline.h>
#include <GLibMemory.h>
#include <GLibWindow.h>
#include <GLibBinaryLoader.h>
#include <GLibVertexBuffer.h>

/* pragma link */
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace glib
{

    /* constants */
    const int SHADER_MAX = 10;
    const int BACKBUFF_MAX = 2;

    /* device interfaces */
    glib::GLibDescriptorPool*           pDescriptorPool;
    glib::GLibPipeline*                 pPipelines[SHADER_MAX];
    glib::GLibPipeline*                 pCurrentPipeline;
    glib::GLibGraphicsCommandList*      pGraphicsCommandLists[SHADER_MAX];
    glib::GLibDevice*                   pDevice;
    glib::GLibSwapChain*                pSwapChain;
    glib::GLibCommandAllocator*         pCommandAllocator;
    glib::GLibCommandQueue*             pCommandQueue;
    glib::GLibFence*                    pFence;
    glib::GLibTime*                     pTime;
    glib::GLibWindow*                   pWindow;

    /* resources */
    glib::GLibVertexBuffer*             pVertexBuffer;


    D3D12_VIEWPORT                      ViewPort;
    D3D12_RECT                          ScissorRect;
}

bool glib::Init()
{
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
    glib::Logger::CriticalLog("GLib initialize begin.");
    {
        pWindow                             = new GLibWindow();
        pDescriptorPool                     = new GLibDescriptorPool;
        for (int i = 0; i < SHADER_MAX; ++i)
        {
            pPipelines[i]            = new GLibPipeline;
            pGraphicsCommandLists[i] = new GLibGraphicsCommandList;
        }
        pVertexBuffer                       = new glib::GLibVertexBuffer;
        pDevice                             = new GLibDevice;
        pSwapChain                          = new GLibSwapChain;
        pCommandAllocator                   = new GLibCommandAllocator;
        pCommandQueue                       = new GLibCommandQueue;
        pFence                              = new GLibFence;
        pTime                               = new GLibTime;

        ViewPort                            = { 0.0f, 0.0f, static_cast<float>(pWindow->GetClientWidth()), static_cast<float>(pWindow->GetClientHeight()), 0.0f, 1.0f };
        ScissorRect                         = { 0, 0, pWindow->GetClientWidth(), pWindow->GetClientHeight() };
    }

    // Initialize the window
    pWindow->Finalize(pWindow->GetName(), pWindow->GetClientWidth(), pWindow->GetClientHeight());

    if (!pWindow->GetHWnd())
    {
        glib::Logger::ErrorLog("Failed to create window.");
        return false;
    }
    else
    {
        glib::Logger::DebugLog("GLibWindow created successfully: " + glib::StringUtil::WStringToString(pWindow->GetName()));
    }

    // Enable the debug layer
    glib::GLibDebug::EnableDebugLayer();
    /*
    Device
    |
    Allocator
    |
    CommandList
    |
    CommandQueue
    |
    Fence
    |
    DescriptorPool
    |
    SwapChain
    |
    RootSignature
    |
    Pipeline
    |
    Time
    |
    ImGui
    */

    // Initialize the device
    pDevice->Initialize(D3D_FEATURE_LEVEL_12_0);


    // Initialize the command allocator
    pCommandAllocator->Initialize(pDevice->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);


    // Initialize the command list
    pGraphicsCommandLists[0]->Initialize(pDevice->Get(), pCommandAllocator->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);


    // Initialize the command queue
    D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
    cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    pCommandQueue->Initialize(pDevice->Get(), cmdQueueDesc);


    // Initialize the fence
    pFence->Initialize(pDevice, pCommandQueue, D3D12_FENCE_FLAG_NONE);


    // Initialize the descriptor pool
    if (!pDescriptorPool->Initialize(pDevice->Get()))
    {
        glib::Logger::ErrorLog("Failed to initialize descriptor pool.");
        return false;
    }


    // Initialize the descriptor pool
    pDescriptorPool->Initialize(pDevice->Get());


    // Initialize the swap chain
    if (!pSwapChain->Initialize(pDevice, pCommandQueue, pCommandAllocator, pDescriptorPool, BACKBUFF_MAX))
    {
        glib::Logger::ErrorLog("Failed to initialize swap chain.");
        return false;
    }


    // Initialize the RootSignature
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSigDesc.NumParameters = 0; // No parameters for now
    rootSigDesc.pParameters = nullptr;
    rootSigDesc.NumStaticSamplers = 0; // No static samplers for now
    rootSigDesc.pStaticSamplers = nullptr;

    ID3DBlob* blob;
    HRESULT hr = D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &blob,
        nullptr
    );
    glib::Logger::FormatDebugLog("Serializing root signature...");

    if (FAILED(hr))
    {
        glib::Logger::FormatErrorLog("Failed to serialize root signature: HRESULT = 0x{:X}", hr);
        return false;
    }

    ComPtr<ID3D12RootSignature> pRootsignature;
    hr = pDevice->Get()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(pRootsignature.GetAddressOf()));
    glib::Logger::FormatDebugLog("Creating root signature...");

    if (FAILED(hr))
    {
        glib::Logger::FormatErrorLog("Failed to create root signature: HRESULT = 0x{:X}", hr);
        blob->Release();
        return false;
    }

    blob->Release();
    glib::Logger::FormatDebugLog("Root signature created successfully.");


    // Initialize the pipelines
    // ここでやっている複数の設定(DESC)は全て、一つのパイプラインの設定をしています。
    // 最終的にまとめてようやく、一つのパイプラインが出来上がります。
    // 細かな設定は、パイプラインの種類によって異なるので、いろいろ調べつつ、自分が求めているパイプラインにカスタムしよう。
#ifdef _DEBUG
    glib::GLibBinaryLoader vs("x64/Debug/SpriteVS.cso");
    glib::GLibBinaryLoader ps("x64/Debug/SpritePS.cso");
#else
    glib::GLibBinaryLoader vs("x64/Release/SpriteVS.cso");
    glib::GLibBinaryLoader ps("x64/Release/SpritePS.cso");
#endif

    UINT slot0 = 0;
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        //{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } あとで追加
    };
    D3D12_RASTERIZER_DESC rastDesc = {};
    rastDesc.FrontCounterClockwise = false;
    rastDesc.CullMode = D3D12_CULL_MODE_NONE;
    rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rastDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rastDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rastDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rastDesc.DepthClipEnable = true;
    rastDesc.MultisampleEnable = false;
    rastDesc.AntialiasedLineEnable = false;
    rastDesc.ForcedSampleCount = 0;
    rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = false;
    blendDesc.RenderTarget[0].LogicOpEnable = false;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = false; // Depth test is disabled for now
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencilDesc.StencilEnable = false; // Stencil test is disabled for now
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = pRootsignature.Get();
    psoDesc.VS = { vs.Code(), vs.Size() };
    psoDesc.PS = { ps.Code(), ps.Size() };
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.RasterizerState = rastDesc;
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT; // Depth-stencil format
    psoDesc.SampleMask = UINT_MAX; // Sample mask for blending
    psoDesc.SampleDesc.Count = 1; // No multisampling
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // Primitive topology type
    psoDesc.NumRenderTargets = 1; // Number of render targets
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Render target format
    pPipelines[0]->Initialize(pDevice->Get(), psoDesc);


    // Initialize the vertex buffer
    float vertices[] = {
        -0.5f, -0.3f, 0.0f,
        0.3f, 0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,

        -0.3f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, 0.3f, 0.0f
    };

    UINT vertexCount = _countof(vertices) / 3;
    UINT stride = sizeof(float) * 3;
    size_t size = sizeof(vertices);

    pVertexBuffer->Initialize(pDevice->Get(), vertices, size, vertexCount, stride);




    // Initialize the time management
    pTime->SetLevelLoaded();


    glib::Logger::DebugLog("GLib initialized successfully.");
    glib::Logger::CriticalLog("GLib initialize end.");
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
    return true;
}

void glib::BeginRender(const GLIB_PIPELINE_TYPE& usePipelineType)
{
    // 描画受付開始 (DrawBegin内でRenderTargetのクリア・セット、バリアの設定を行っています。)
    pSwapChain->DrawBegin(pGraphicsCommandLists[usePipelineType]);

    // 以下、パイプラインの設定と、描画する頂点のセットを行います。
    pCurrentPipeline = pPipelines[usePipelineType];
    pGraphicsCommandLists[usePipelineType]->Get()->SetPipelineState(pCurrentPipeline->Get());
    pGraphicsCommandLists[usePipelineType]->Get()->RSSetViewports(1, &ViewPort);
    pGraphicsCommandLists[usePipelineType]->Get()->RSSetScissorRects(1, &ScissorRect);
    pGraphicsCommandLists[usePipelineType]->Get()->SetGraphicsRootSignature(pCurrentPipeline->GetRootSignature());

    // 頂点セット
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView[] =
    {
        pVertexBuffer->GetVertexBufferView()
    };
    UINT vertexBufferCount = _countof(vertexBufferView);
    pGraphicsCommandLists[usePipelineType]->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pGraphicsCommandLists[usePipelineType]->Get()->IASetVertexBuffers(0, vertexBufferCount, vertexBufferView);

    // 描画
    pGraphicsCommandLists[usePipelineType]->Get()->DrawInstanced(pVertexBuffer->GetVertexCount(), 1, 0, 0);
}

void glib::EndRender(const GLIB_PIPELINE_TYPE& usePipelineType)
{
    pSwapChain->DrawEnd(pGraphicsCommandLists[usePipelineType]);
    WaitDrawDone();
}

void glib::Release()
{
    WaitDrawDone();
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
    glib::Logger::CriticalLog("GLib release begin.");

    // D3D12 Release
    /*
    Device
    |
    Allocator
    |
    CommandList
    |
    CommandQueue
    |
    Fence
    |
    DescriptorPool
    |
    SwapChain
    |
    RootSignature
    |
    Pipeline
    |
    VertexBuffer
    |
    Time
    |
    ImGui
    */

    SafeDelete(pTime);
    SafeDelete(pVertexBuffer);
    for (int i = 0; i < SHADER_MAX; ++i)
    {
        SafeDelete(pPipelines[i]);
    }
    SafeDelete(pSwapChain);
    SafeDelete(pDescriptorPool);
    SafeDelete(pFence);
    SafeDelete(pCommandQueue);
    for (int i = 0; i < SHADER_MAX; ++i)
    {
        SafeDelete(pGraphicsCommandLists[i]);
    }
    SafeDelete(pCommandAllocator);
    SafeDelete(pDevice);
    
    // Release the window
    if (pWindow->GetHWnd())
    {
        DestroyWindow(pWindow->GetHWnd());
        SafeDelete(pWindow);
    }

    // Log release message
    glib::Logger::DebugLog("GLib released successfully.");
    glib::Logger::CriticalLog("GLib release end.");
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
}

void glib::WaitDrawDone()
{
    pFence->WaitDrawDone();
}

void glib::RefreshDeltaTime()
{
    pTime->Update();
}

float glib::DeltaTime()
{
    return pTime->DeltaTime();
}

void glib::ShowWindow()
{
    pWindow->Finalize(pWindow->GetName(), pWindow->GetClientWidth(), pWindow->GetClientHeight());
    if (pWindow->GetHWnd())
    {
        ::ShowWindow(pWindow->GetHWnd(), SW_SHOW);
        UpdateWindow(pWindow->GetHWnd());
    }
}

void glib::SetWindowPos(int x, int y)
{
    pWindow->SetPos(x, y);
}

void glib::SetWindowAspect(float aspect)
{
    pWindow->SetAspect(aspect);
}

void glib::SetWindowStyle(const GLIB_WINDOW_STYLE& style)
{
    pWindow->SetStyle(static_cast<DWORD>(style));
}

void glib::SetWindowName(const LPCWSTR& wndName)
{
    pWindow->SetName(wndName);
}

void glib::SetWindowSize(int width, int height)
{
    pWindow->SetClientWidth(width);
    pWindow->SetClientHeight(height);

    if (pWindow->GetHWnd())
    {
        RECT rect;
        GetClientRect(pWindow->GetHWnd(), &rect);
        int newWidth = width;
        int newHeight = height;
        SetWindowPos(pWindow->GetHWnd(), nullptr, 0, 0, newWidth, newHeight, SWP_NOMOVE | SWP_NOZORDER);
    }
}

glib::GLibWindow* glib::GetWindow()
{
    return pWindow;
}
