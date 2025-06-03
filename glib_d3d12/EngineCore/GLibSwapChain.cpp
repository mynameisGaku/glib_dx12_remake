#include <GLibSwapChain.h>
#include <GLibLogger.h>
#include <GLibWindow.h>
#include <GLibCommandQueue.h>
#include <GLibDescriptorPool.h>
#include <GLibGraphicsCommandList.h>
#include <GLibCommandAllocator.h>
#include <GLibFence.h>
#include <GLibDevice.h>
#include <GLib.h>

glib::GLibSwapChain* glib::GLibSwapChain::m_Instance = nullptr;

bool glib::GLibSwapChain::Initialize(ID3D12Device* device, UINT buffIdx)
{
    // Swapchain作る
    {
        IDXGIFactory4* factory = nullptr;
        m_Hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
        if (FAILED(m_Hr))
        {
            glib::Logger::FormatErrorLog("Failed to create DXGIFactory. HRESULT: 0x{%X}", m_Hr);
            return false;
        }
        glib::Logger::DebugLog("DXGIFactory created successfully.");

        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.BufferCount = buffIdx; // バックバッファの数
        desc.Width = glib::Window::ClientWidth; // ウィンドウの幅
        desc.Height = glib::Window::ClientHeight; // ウィンドウの高さ
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // バックバッファのフォーマット
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // レンダーターゲットとして使用
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // スワップ効果
        desc.SampleDesc.Count = 1; // マルチサンプルなし
        desc.SampleDesc.Quality = 0; // マルチサンプルの品質

        IDXGISwapChain1* swapChain = nullptr;
        m_Hr = factory->CreateSwapChainForHwnd(
            glib::GLibCommandQueue::GetInstance().Get(),
            glib::Window::m_HWnd,
            &desc,
            nullptr,
            nullptr,
            &swapChain
        );

        if (FAILED(m_Hr))
        {
            glib::Logger::FormatErrorLog("Failed to create swap chain. HRESULT: 0x{%X}", m_Hr);
            factory->Release();
            return false;
        }

        // Query IDXGISwapChain4 Interface
        m_Hr = swapChain->QueryInterface(IID_PPV_ARGS(m_SwapChain.GetAddressOf()));
        if (FAILED(m_Hr))
        {
            glib::Logger::FormatErrorLog("Failed to query IDXGISwapChain4 interface. HRESULT: 0x{%X}", m_Hr);
            swapChain->Release();
            factory->Release();
            return false;
        }
        swapChain->Release(); // QueryInterfaceで取得したので、ここではReleaseする
        factory->Release(); // FactoryもRelease
    }

    glib::Logger::DebugLog("Swap chain created successfully.");

    // バックバッファビューの入れ物であるDescriptorHeapを作成
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.NumDescriptors = buffIdx; // バックバッファの数
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー用のヒープ
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // シェーダーからアクセスしない
        if (!m_BbvHeap.Initialize(glib::GLibDescriptorPool::GetInstance().Allocate("SwapChainRTVHeap", desc)))
        {
            glib::Logger::FormatErrorLog("Failed to create RTV descriptor heap. HRESULT: 0x{%X}", m_Hr);
            return false;
        }
        glib::Logger::DebugLog("RTV descriptor heap created successfully.");
    }

    // バックバッファビューをディスクリプタヒープに作る
    {
        D3D12_CPU_DESCRIPTOR_HANDLE hBbvHeap = m_BbvHeap.Get()->GetCPUDescriptorHandleForHeapStart();
        m_BbvHeapSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        for (UINT i = 0; i < buffIdx; ++i)
        {
            m_BackBuffers.push_back(nullptr); // バックバッファの初期化
            m_Hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i]));
            if (FAILED(m_Hr))
            {
                glib::Logger::FormatErrorLog("Failed to get swap chain buffer {}. HRESULT: 0x{%X}", i, m_Hr);
                return false;
            }
            hBbvHeap.ptr += i * m_BbvHeapSize;
            device->CreateRenderTargetView(m_BackBuffers[i].Get(), nullptr, hBbvHeap);
            glib::Logger::FormatDebugLog("Render target view for buffer {%d} created successfully.", i);
        }
    }

    glib::Logger::DebugLog("All render target views created successfully.");

    return true;
}

void glib::GLibSwapChain::DrawBegin()
{
    // バックバッファをクリアする
    {
        glib::GLibGraphicsCommandList::GetInstance().Get()->Reset(glib::GLibCommandAllocator::GetInstance().Get(), nullptr);

        m_BackBufIdx = m_SwapChain->GetCurrentBackBufferIndex();

        // バリアでバックバッファを描画ターゲットに切り替える
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_BackBuffers[m_BackBufIdx].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        glib::GLibGraphicsCommandList::GetInstance().Get()->ResourceBarrier(1, &barrier);

        // バックバッファの場所を指すディスクリプタヒープハンドルを用意する
        auto hBbvHeap = m_BbvHeap.Get()->GetCPUDescriptorHandleForHeapStart();
        hBbvHeap.ptr += m_BackBufIdx * m_BbvHeapSize;
        // バックバッファを描画ターゲットとして設定する
        glib::GLibGraphicsCommandList::GetInstance().Get()->OMSetRenderTargets(1, &hBbvHeap, FALSE, nullptr);
        // 描画ターゲットのクリア
        static float radian = .0f;
        float r = cos(radian) * 0.5f + 0.5f;
        float g = 0.25f;
        float b = 0.5f;
        const float clearColor[4] = { r, g, b, 1.0f };
        radian += 0.01f; // 色を変えるために少しずつradianを増やす
        glib::GLibGraphicsCommandList::GetInstance().Get()->ClearRenderTargetView(hBbvHeap, clearColor, 0, nullptr);
    }
}

void glib::GLibSwapChain::DrawEnd()
{
    // バリアでバックバッファを表示用に切り替える
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_BackBuffers[m_BackBufIdx].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    glib::GLibGraphicsCommandList::GetInstance().Get()->ResourceBarrier(1, &barrier);

    // コマンドリストクローズ
    m_Hr = glib::GLibGraphicsCommandList::GetInstance().Get()->Close();
    if (FAILED(m_Hr))
    {
        glib::Logger::FormatErrorLog("Failed to close command list. HRESULT: 0x{%X}", m_Hr);
        return;
    }
    glib::Logger::DebugLog("Command list closed successfully.");

    // コマンドリストを実行
    ID3D12CommandList* commandLists[] = { glib::GLibGraphicsCommandList::GetInstance().Get() };
    glib::GLibCommandQueue::GetInstance().Get()->ExecuteCommandLists(_countof(commandLists), commandLists);

    // 描画完了を待つ
    glib::WaitDrawDone();

    // バックバッファを表示
    // Present
    int vsync = m_BackBufIdx == 3 ? 0 : 1;
    m_Hr = m_SwapChain->Present(vsync, 0); // VSyncを有効にして表示
    if (FAILED(m_Hr))
    {
        glib::Logger::FormatErrorLog("Failed to present swap chain. HRESULT: 0x{%X}", m_Hr);
        return;
    }
    glib::Logger::DebugLog("Swap chain presented successfully.");

    // コマンドアロケータを待つ
    m_Hr = glib::GLibCommandAllocator::GetInstance().Get()->Reset();
    if (FAILED(m_Hr))
    {
        glib::Logger::FormatErrorLog("Failed to reset command allocator. HRESULT: 0x{&X}", m_Hr);
        return;
    }
    glib::Logger::DebugLog("Command allocator reset successfully.");
}
