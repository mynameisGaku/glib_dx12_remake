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

/* pragma link */
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace glib
{
    const int SHADER_MAX = 10;
    const int BACKBUFF_MAX = 2;
    glib::GLibDescriptorPool*           pDescriptorPool;
    glib::GLibPipeline*                 pPipeline[SHADER_MAX];
    glib::GLibGraphicsCommandList*      pGraphicsCommandList[SHADER_MAX];
    glib::GLibDevice*                   pDevice;
    glib::GLibSwapChain*                pSwapChain;
    glib::GLibCommandAllocator*         pCommandAllocator;
    glib::GLibCommandQueue*             pCommandQueue;
    glib::GLibFence*                    pFence;
    glib::GLibTime*                     pTime;
    glib::GLibWindow*                   pWindow;
}

bool glib::Init()
{
    {
        pWindow                             = new GLibWindow();
        pDescriptorPool                     = new GLibDescriptorPool;
        for (int i = 0; i < SHADER_MAX; ++i)
        {
            pPipeline[i]            = new GLibPipeline;
            pGraphicsCommandList[i] = new GLibGraphicsCommandList;
        }
        pDevice                             = new GLibDevice;
        pSwapChain                          = new GLibSwapChain;
        pCommandAllocator                   = new GLibCommandAllocator;
        pCommandQueue                       = new GLibCommandQueue;
        pFence                              = new GLibFence;
        pTime                               = new GLibTime;
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
    pGraphicsCommandList[0]->Initialize(pDevice->Get(), pCommandAllocator->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);


    // Initialize the command queue
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    pCommandQueue->Initialize(pDevice->Get(), desc);


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

    // Initialize the time management
    pTime->SetLevelLoaded();


    glib::Logger::CriticalLog("GLib initialized successfully.");
    return true;
}

void glib::BeginRender()
{
    pSwapChain->DrawBegin(pGraphicsCommandList[0]);
}

void glib::EndRender()
{
    pSwapChain->DrawEnd(pGraphicsCommandList[0]);
    WaitDrawDone();
}

void glib::Release()
{
    WaitDrawDone();

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
    Pipeline
    |
    Time
    |
    ImGui
    */

    SafeDelete(pTime);
    for (int i = 0; i < SHADER_MAX; ++i)
    {
        SafeDelete(pPipeline[i]);
    }
    SafeDelete(pSwapChain);
    SafeDelete(pDescriptorPool);
    SafeDelete(pFence);
    SafeDelete(pCommandQueue);
    for (int i = 0; i < SHADER_MAX; ++i)
    {
        SafeDelete(pGraphicsCommandList[i]);
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
    glib::Logger::CriticalLog("GLib released successfully.");
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
