#include <GLib.h>
#include <GLibDevice.h>
#include <GLibDebug.h>
#include <GLibCommandAllocator.h>
#include <GLibCommandQueue.h>
#include <GLibGraphicsCommandList.h>
#include <GLibFence.h>
#include <GLibStringUtil.h>
#include <GLibDescriptorHeap.h>
#include <GLibDescriptorPool.h>
#include <GLibSwapChain.h>
#include <GLibTime.h>
#include <GLibPipeline.h>
#include <GLibMemory.h>

/* imgui */
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_impl_win32.h>
#include <ImGUI/imgui_impl_dx12.h>

/* pragma link */
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace glib
{
    const int SHADER_MAX = 10;
    const int BACKBUFF_MAX = 2;
    GLibDescriptorPool*         pDescriptorPool;
    GLibPipeline                pPipeline[SHADER_MAX];
    GLibGraphicsCommandList     GraphicsCommandList[SHADER_MAX];
    glib::GLibSwapChain         SwapChain;
}

bool glib::Init()
{
    // Initialize the window
    glib::Window::Finalize(glib::Window::m_WindowTitle, glib::Window::ClientWidth, glib::Window::ClientHeight);

    if (!glib::Window::m_HWnd)
    {
        glib::Logger::ErrorLog("Failed to create window.");
        return false;
    }
    else
    {
        glib::Logger::DebugLog("Window created successfully: " + glib::StringUtil::WStringToString(glib::Window::m_WindowTitle));
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
    Pipeline
    |
    Time
    |
    ImGui
    */

    // Initialize the device
    glib::GLibDevice::GetInstance().Initialize(D3D_FEATURE_LEVEL_12_0);


    // Initialize the command allocator
    glib::GLibCommandAllocator::GetInstance().Initialize(glib::GLibDevice::GetInstance().Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);


    // Initialize the command list
    GraphicsCommandList[0].Initialize(glib::GLibDevice::GetInstance().Get(), glib::GLibCommandAllocator::GetInstance().Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);


    // Initialize the command queue
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    glib::GLibCommandQueue::GetInstance().Initialize(glib::GLibDevice::GetInstance().Get(), desc);


    // Initialize the fence
    glib::GLibFence::GetInstance().Initialize(glib::GLibDevice::GetInstance().Get(), D3D12_FENCE_FLAG_NONE);


    // Initialize the descriptor pool
    if (!glib::GLibDescriptorPool::GetInstance().Initialize(glib::GLibDevice::GetInstance().Get()))
    {
        glib::Logger::ErrorLog("Failed to initialize descriptor pool.");
        return false;
    }


    // Initialize the descriptor pool
    glib::GLibDescriptorPool::GetInstance().Initialize(glib::GLibDevice::GetInstance().Get());


    // Initialize the swap chain
    if (!SwapChain.Initialize(glib::GLibDevice::GetInstance().Get(), 2))
    {
        glib::Logger::ErrorLog("Failed to initialize swap chain.");
        return false;
    }

    // Initialize the time management
    glib::GLibTime::GetInstance().SetLevelLoaded();

    // imgui
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        srvHeapDesc.NumDescriptors = 1000; // Number of descriptors in the heap
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Shader Resource View heap type
        glib::GLibDescriptorPool::GetInstance().Allocate("ImGuiSRVHeap", srvHeapDesc);

        auto imguiHeap = glib::GLibDescriptorPool::GetInstance().Get("ImGuiSRVHeap");

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(glib::Window::m_HWnd);
        ImGui_ImplDX12_Init(glib::GLibDevice::GetInstance().Get(), 2,
            DXGI_FORMAT_R8G8B8A8_UNORM, imguiHeap,
            imguiHeap->GetCPUDescriptorHandleForHeapStart(),
            imguiHeap->GetGPUDescriptorHandleForHeapStart());
    }

    glib::Logger::CriticalLog("GLib initialized successfully.");
    return true;
}

void glib::BeginRender()
{
    RECT rect;
    GetClientRect(glib::Window::m_HWnd, &rect);

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame((float)(rect.right - rect.left) / glib::Window::ClientWidth, (float)(rect.bottom - rect.top) / (float)glib::Window::ClientHeight, (float)glib::Window::ClientWidth, (float)glib::Window::ClientHeight);
    ImGui::NewFrame();
    SwapChain.DrawBegin(&GraphicsCommandList[0]);
}

void glib::EndRender()
{
    ImGui::Render();

    SwapChain.DrawEnd(&GraphicsCommandList[0]);
    WaitDrawDone();
}

void glib::Release()
{
    WaitDrawDone();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    glib::GLibDescriptorPool::GetInstance().Free("ImGuiSRVHeap");

    // D3D12 Release
    /*
    SwapChain
    |
    DescriptorPool
    |
    Fence
    |
    CommandQueue
    |
    CommandList
    |
    Allocator
    |
    Device
    */
    glib::GLibDescriptorPool::GetInstance().Release();
    glib::GLibFence::GetInstance().Release();
    glib::GLibCommandQueue::GetInstance().Release();
    glib::GLibCommandAllocator::GetInstance().Release();
    glib::GLibDevice::GetInstance().Release();

    // Release the window
    if (glib::Window::m_HWnd)
    {
        DestroyWindow(glib::Window::m_HWnd);
        glib::Window::m_HWnd = nullptr;
    }

    glib::GLibTime::GetInstance().Destroy();

    // Log release message
    glib::Logger::CriticalLog("GLib released successfully.");
}

void glib::WaitDrawDone()
{
    glib::GLibFence::GetInstance().WaitDrawDone();
}

void glib::RefreshDeltaTime()
{
    glib::GLibTime::GetInstance().Update();
}

float glib::DeltaTime()
{
    return glib::GLibTime::GetInstance().DeltaTime();
}

void glib::ShowWindow()
{
    glib::Window::Finalize(glib::Window::m_WindowTitle, glib::Window::ClientWidth, glib::Window::ClientHeight);
    if (glib::Window::m_HWnd)
    {
        ::ShowWindow(glib::Window::m_HWnd, SW_SHOW);
        UpdateWindow(glib::Window::m_HWnd);
    }
}

void glib::SetWindowPos(int x, int y)
{
    glib::Window::SetPos(x, y);
}

void glib::SetWindowAspect(float aspect)
{
    glib::Window::SetAspect(aspect);
}

void glib::SetWindowStyle(const GLIB_WINDOW_STYLE& style)
{
    glib::Window::SetStyle(static_cast<DWORD>(style));
}

void glib::SetWindowName(const LPCWSTR& wndName)
{
    glib::Window::SetName(wndName);
}

void glib::SetWindowSize(int width, int height)
{
    glib::Window::ClientWidth = width;
    glib::Window::ClientHeight = height;

    if (glib::Window::m_HWnd)
    {
        RECT rect;
        GetClientRect(glib::Window::m_HWnd, &rect);
        int newWidth = width;
        int newHeight = height;
        SetWindowPos(glib::Window::m_HWnd, nullptr, 0, 0, newWidth, newHeight, SWP_NOMOVE | SWP_NOZORDER);
    }
}
