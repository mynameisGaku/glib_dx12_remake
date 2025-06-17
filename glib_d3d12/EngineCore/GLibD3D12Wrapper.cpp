#include "GLibD3D12Wrapper.h"
#include "GLibWindow.h"
#include "GLibMemory.h"
#include "GLibDebug.h"

#include <cassert>

glib::GLibD3D12Wrapper::GLibD3D12Wrapper()
{
    assert(init());
}

glib::GLibD3D12Wrapper::~GLibD3D12Wrapper()
{
    term();
}

bool glib::GLibD3D12Wrapper::init()
{
    assert(initD3D());

    {
        // 頂点データ
        Vertex vertices[] =
        {
            { XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
            { XMFLOAT3( 1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
            { XMFLOAT3( 0.0f,  1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        };

        // ヒーププロパティ
        D3D12_HEAP_PROPERTIES prop{};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // リソース設定
        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = sizeof(vertices);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // リソース生成
        HRESULT hr{};
        hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_VB.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create resource. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created resource.");

        // マップ
        void* ptr = nullptr;
        hr = m_VB->Map(0, nullptr, &ptr);
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to mapped resource. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully mapped resource.");

        // 頂点データをマップ先に
        memcpy(ptr, vertices, sizeof(vertices));

        // マップ解除
        m_VB->Unmap(0, nullptr);

        // 頂点バッファビューの設定
        m_VBV.BufferLocation = m_VB->GetGPUVirtualAddress();
        m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));
        m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));
    }

    // ディスクリプタヒープの生成
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 5000;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;

        HRESULT hr{};
        hr = m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create descriptor heap. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully create descriptor heap.");
    }

    // 定数バッファ生成
    {
        D3D12_HEAP_PROPERTIES prop{};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // 設定
        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = sizeof(Transform);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        auto incrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        for (int i = 0; i < m_FrameCount; i++)
        {
            // リソース生成
            auto hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_CBVs[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("Failed to create resource. HRESULT=0x%x", hr);
                return false;
            }
            glib::Logger::FormatDebugLog("Successfully create resource.");
        }
    }

    return true;
}

void glib::GLibD3D12Wrapper::term()
{
    termD3D();
}

bool glib::GLibD3D12Wrapper::initD3D()
{
    /*
    デバイス
    ↓
    コマンドキュー
    ↓
    スワップチェイン
    ↓
    コマンドアロケータ
    ↓
    コマンドリスト
    ↓
    レンダーターゲットビュー
    ↓
    フェンス
    */

    HRESULT hr{};

    // Enable the debug layer
    glib::GLibDebug::EnableDebugLayer();

    // デバイス
    {
        hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create d3d12 device. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created d3d12 device.");
    }

    // コマンドキュー
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Type       = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Priority   = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags      = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask   = 0;
        hr = m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create d3d12 command queue. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created d3d12 command queue.");
    }

    // スワップチェイン
    {
        // DXGIファクトリーの生成
        GLibComPtr<IDXGIFactory4> factory{};
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create dxgi factory4. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created dxgi factory4.");

        // スワップチェインの設定
        DXGI_SWAP_CHAIN_DESC desc{};

        m_FrameCount = (UINT32)glib::GetBackBufferCount();

        desc.BufferDesc.Width                       = (UINT)glib::GetWindow()->GetClientWidth();
        desc.BufferDesc.Height                      = (UINT)glib::GetWindow()->GetClientHeight();
        desc.BufferDesc.RefreshRate.Numerator       = (UINT)glib::GetMaxFPS();
        desc.BufferDesc.RefreshRate.Denominator     = 1;
        desc.BufferDesc.ScanlineOrdering            = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        desc.BufferDesc.Scaling                     = DXGI_MODE_SCALING_UNSPECIFIED;
        desc.BufferDesc.Format                      = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count                       = 1;
        desc.SampleDesc.Quality                     = 0;
        desc.BufferUsage                            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount                            = m_FrameCount;
        desc.OutputWindow                           = glib::GetWindow()->GetHWnd();
        desc.Windowed                               = TRUE;
        desc.SwapEffect                             = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags                                  = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // スワップチェインの生成
        IDXGISwapChain* swapChain{};

        hr = factory->CreateSwapChain(m_CommandQueue.Get(), &desc, &swapChain);
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create dxgi swapchain. HRESULT=0x%x", hr);
            factory.Reset();
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created dxgi swapchain.");

        // IDXGISwapChain3を取得
        hr = swapChain->QueryInterface(IID_PPV_ARGS(m_SwapChain.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create dxgi swapchain3. HRESULT=0x%x", hr);
            factory.Reset();
            glib::SafeReleaseDX(swapChain);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created dxgi swapchain3.");

        // バックバッファ番号を取得
        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        // 不要なので削除
        factory.Reset();
        glib::SafeReleaseDX(swapChain);
    }

    // コマンドアロケーター
    {
        for (UINT32 i = 0; i < m_FrameCount; i++)
        {
            m_CommandAllocators.push_back(GLibComPtr<ID3D12CommandAllocator>());
            hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocators[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("Failed to create d3d12 command allocator [%d]. HRESULT=0x%x", i, hr);
                return false;
            }
            glib::Logger::FormatDebugLog("Successfully created d3d12 command allocator.");
        }
    }

    // コマンドリスト
    {
        hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[m_FrameIndex].Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create d3d12 command list. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created d3d12 command list.");
    }

    // レンダーターゲットビュー
    {
        // ディスクリプタヒープの設定
        D3D12_DESCRIPTOR_HEAP_DESC desc{};

        desc.NumDescriptors = m_FrameCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;

        for (int i = 0; i < GLIB_DESCRIPTOR_HEAP_TYPE_MAX; i++)
        {
            m_DescriptorHeaps.push_back(GLibComPtr<ID3D12DescriptorHeap>());
        }

        // ディスクリプタヒープ生成
        hr = m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_RTV].GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create d3d12 rtv descriptor heap. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created d3d12 rtv descriptor heap.");

        auto handle = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_RTV]->GetCPUDescriptorHandleForHeapStart();
        auto incrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (auto i = 0u; i < m_FrameCount; i++)
        {
            m_ColorBuffers.push_back(GLibComPtr<ID3D12Resource>());
            hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_ColorBuffers[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("Failed to retrieve buffer from swap chain at index [%d]. HRESULT=0x%08X", i, hr);
                return false;
            }
            glib::Logger::FormatDebugLog("Successfully retrieved buffer from swap chain and created RTV descriptor.");

            D3D12_RENDER_TARGET_VIEW_DESC viewDesc{};
            viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipSlice = 0;
            viewDesc.Texture2D.PlaneSlice = 0;

            // レンダーターゲットビュー生成
            m_Device->CreateRenderTargetView(m_ColorBuffers[i].Get(), &viewDesc, handle);

            m_HandlesRTV.push_back(D3D12_CPU_DESCRIPTOR_HANDLE());
            m_HandlesRTV[i] = handle;
            handle.ptr += incrementSize;
        }
    }

    // フェンス
    {
        // カウンターリセット
        for (auto i = 0u; i < m_FrameCount; i++)
        {
            m_FenceCounters.push_back(UINT64());
            m_FenceCounters[i] = 0;
            glib::Logger::FormatDebugLog("Successfully fence counters[%d] reset.", i);
        }

        // フェンス生成  
        hr = m_Device->CreateFence(m_FenceCounters[m_FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));  
        if (FAILED(hr))  
        {  
            glib::Logger::FormatCriticalLog("Failed to create d3d12 fence. HRESULT=0x%08X", hr);  
            return false;  
        }  
        glib::Logger::FormatDebugLog("Successfully created d3d12 fence.");

        m_FenceCounters[m_FrameIndex]++;

        // イベント生成  
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);  
        if (m_FenceEvent == nullptr)  
        {  
           glib::Logger::FormatCriticalLog("Failed to create fence event.");  
           return false;  
        }  
        glib::Logger::FormatDebugLog("Successfully created fence event.");
    }

    // コマンドリスト閉じる
    m_CommandList->Close();

    return true;
}

void glib::GLibD3D12Wrapper::waitGpu()
{
    assert(m_CommandQueue);
    assert(m_Fence);
    assert(m_FenceEvent);

    // シグナル
    m_CommandQueue->Signal(m_Fence.Get(), m_FenceCounters[m_FrameIndex]);

    // 完了時にイベントセット
    m_Fence->SetEventOnCompletion(m_FenceCounters[m_FrameIndex], m_FenceEvent);

    // 待機処理
    WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

    // カウンターフヤス
    m_FenceCounters[m_FrameIndex]++;
}

void glib::GLibD3D12Wrapper::present(UINT32 interval)
{
    HRESULT hr{};
    hr = m_SwapChain->Present(interval, 0);

    if (FAILED(hr))
    {
        glib::Logger::FormatWarningLog("Present failed. HRESULT=0x%08X", hr);
    }

    // シグナル
    const auto currentValue = m_FenceCounters[m_FrameIndex];
    m_CommandQueue->Signal(m_Fence.Get(), currentValue);

    // バックバッファ番号更新
    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

    // 次のフレームの描画準備がまだなら待機
    if (m_Fence->GetCompletedValue() < m_FenceCounters[m_FrameIndex])
    {
        m_Fence->SetEventOnCompletion(m_FenceCounters[m_FrameIndex], m_FenceEvent);
        WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
    }

    // 次のフレームのフェンスカウンター増やす
    m_FenceCounters[m_FrameIndex] = currentValue + 1;
}

void glib::GLibD3D12Wrapper::termD3D()
{
    waitGpu();

    CloseHandle(m_FenceEvent);
    m_FenceEvent = nullptr;

    m_Fence.Reset();
    m_CommandList.Reset();
    for (auto& allocator : m_CommandAllocators) allocator.Reset();
    for (auto& resource : m_ColorBuffers) resource.Reset();
    for (auto& heap : m_DescriptorHeaps) heap.Reset();
    m_SwapChain.Reset();
    m_CommandQueue.Reset();
    m_Device.Reset();
}

void glib::GLibD3D12Wrapper::BeginRender()
{
    // コマンド記録開始
    m_CommandAllocators[m_FrameIndex]->Reset();
    m_CommandList->Reset(m_CommandAllocators[m_FrameIndex].Get(), nullptr);

    // リソースバリア（Present → RenderTarget）
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_ColorBuffers[m_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // リソースバリア
    m_CommandList->ResourceBarrier(1, &barrier);

    // レンダーターゲット設定
    m_CommandList->OMSetRenderTargets(1, &m_HandlesRTV[m_FrameIndex], FALSE, nullptr);

    // レンダーターゲットビューのクリア
    m_CommandList->ClearRenderTargetView(m_HandlesRTV[m_FrameIndex], &m_ClearColor.x, 0, nullptr);
}

void glib::GLibD3D12Wrapper::EndRender()
{
    // リソースバリア（RenderTarget → Present）
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_ColorBuffers[m_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_CommandList->ResourceBarrier(1, &barrier);

    // コマンドリストの記録終了
    m_CommandList->Close();

    // 実行
    ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(1, ppCommandLists);

    // Present
    present(1);

    // フェンス同期
    m_CommandQueue->Signal(m_Fence.Get(), m_FenceCounters[m_FrameIndex]);
    m_FenceCounters[m_FrameIndex]++;

    // 次フレームへ
    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

    waitGpu();
}

void glib::GLibD3D12Wrapper::SetClearColor(const XMFLOAT4& color)
{
    m_ClearColor = color;
}
