
// os
#include <windows.h>
#include <cstdint>

// d3d
#include <DirectXMath.h>
#include <d3dx12.h>
#include <d3d12.h>
#include <dxgi1_4.h>

// vendor
#include <Vendor/magic_enum/magic_enum.hpp>

// profiler
#include <psapi.h>

#include <GLib.h>
#include <GLibDebug.h>
#include <GLibStringUtil.h>
#include <GLibTime.h>
#include <GLibMemory.h>
#include <GLibWindow.h>
#include <GLibBinaryLoader.h>
#include <GLibD3D12Wrapper.h>

/* pragma link */
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;

namespace glib
{
    // tick debug
    LARGE_INTEGER freq;
    LARGE_INTEGER start, end;
    bool isRefreshTick = false;

    /* constants */
    const int SHADER_MAX = 10;

    // config
    int MaxFPS = 60;
    int BackBufferCountMax = 2;

    /* device interfaces */
    glib::GLibTime*                     pTime;
    glib::GLibWindow*                   pWindow;
    glib::GLibD3D12Wrapper*             pD3D12;


    D3D12_VIEWPORT                      ViewPort;
    D3D12_RECT                          ScissorRect;


    struct CBUFFER_0
    {
        XMMATRIX Mat;
    };

    struct CBUFFER_1
    {
        XMFLOAT4 Diffuse;
    };
}

bool glib::Init()
{
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
    glib::Logger::CriticalLog("GLib initialize begin.");

    {
        pWindow                             = new GLibWindow;
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

    pD3D12 = new GLibD3D12Wrapper;

    // Initialize the pipelines
    // ここでやっている複数の設定(DESC)は全て、一つのパイプラインの設定をしています。
    // 最終的にまとめてようやく、一つのパイプラインが出来上がります。
    // 細かな設定は、パイプラインの種類によって異なるので、いろいろ調べつつ、自分が求めているパイプラインにカスタムしよう。
//#ifdef _DEBUG
//    glib::GLibBinaryLoader vs("x64/Debug/SpriteVS.cso");
//    glib::GLibBinaryLoader ps("x64/Debug/SpritePS.cso");
//#else
//    glib::GLibBinaryLoader vs("x64/Release/SpriteVS.cso");
//    glib::GLibBinaryLoader ps("x64/Release/SpritePS.cso");
//#endif

    // Initialize the time management
    pTime->SetLevelLoaded();
    glib::Logger::DebugLog("Time initialized successfully.");


    glib::Logger::DebugLog("GLib initialized successfully.");
    glib::Logger::CriticalLog("GLib initialize end.");
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
    return true;
}

void glib::BeginRender(const GLIB_PIPELINE_TYPE& usePipelineType)
{
    // メインループ  
    {
        static float radian = 0.0f;
        float radius = 0.4f;
        radian += 1.0f * pTime->DeltaTime();

        XMVECTOR eye = { 0, 0, -2 };
        XMVECTOR focus = { 0, 0, 0 };
        XMVECTOR up = { 0, 1, 0 };
        XMMATRIX view = XMMatrixLookAtLH(eye, focus, up);
        XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, pWindow->GetAspect(), 0.01f, 10.0f);

        XMMATRIX world = XMMatrixScaling(2.0f, 3.0f, 1.0f) * XMMatrixTranslation(-sin(radian) * radius, 0.0f, -cos(radian) * radius);
        world = XMMatrixScaling(1.0f, 2.0f, 1.0f) * XMMatrixRotationAxis({ 1, 0, 1 }, radian) * XMMatrixTranslation(sin(radian) * radius, 0.0f, cos(radian) * radius);
    }

    // 描画受付開始
    pD3D12->BeginRender();
}

void glib::EndRender(const GLIB_PIPELINE_TYPE& usePipelineType)
{
    pD3D12->EndRender();
}

void glib::Release()
{
    WaitDrawDone();
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
    glib::Logger::CriticalLog("GLib release begin.");

    SafeDelete(pTime);
    glib::Logger::DebugLog("Time released successfully.");

    SafeDelete(pD3D12);
    glib::Logger::DebugLog("D3D12 released successfully.");
    
    // Release the window
    if (pWindow->GetHWnd())
    {
        DestroyWindow(pWindow->GetHWnd());
        SafeDelete(pWindow);
        glib::Logger::DebugLog("Window released successfully.");
    }

    // Log release message
    glib::Logger::DebugLog("GLib released successfully.");
    glib::Logger::CriticalLog("GLib release end.");
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
}

void glib::WaitDrawDone()
{

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

void glib::SetBackBufferCount(int count)
{
    BackBufferCountMax = count;
}

int glib::GetBackBufferCount()
{
    return BackBufferCountMax;
}

std::wstring glib::StringToWString(const std::string& str)
{
    std::wstring ret;
    //一度目の呼び出しは文字列数を知るため
    auto result = MultiByteToWideChar(CP_UTF8,
        0,
        str.c_str(),//入力文字列
        (int)str.length(),
        nullptr,
        0);
    assert(result >= 0);
    ret.resize(result);//確保する
    //二度目の呼び出しは変換
    result = MultiByteToWideChar(CP_UTF8,
        0,
        str.c_str(),//入力文字列
        (int)str.length(),
        ret.data(),
        (int)ret.size());
    return ret;
}

std::string glib::WStringToString(const std::wstring& wstr)
{
    std::string ret;
    //一度目の呼び出しは文字列数を知るため
    auto result = WideCharToMultiByte(
        CP_ACP,
        0,
        wstr.c_str(),//入力文字列
        (int)wstr.length(),
        nullptr,
        0,
        nullptr,
        nullptr);
    assert(result >= 0);
    ret.resize(result);//確保する
    //二度目の呼び出しは変換
    result = WideCharToMultiByte(
        CP_ACP,
        0,
        wstr.c_str(),//入力文字列
        (int)wstr.length(),
        ret.data(),
        (int)ret.size(),
        nullptr,
        nullptr);
    return ret;
}

void glib::RunProfile()
{
    std::string title = WStringToString(pWindow->GetDefaultName());

    // Memory profiling  
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
    DWORDLONG virtualMemUsed = memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    SIZE_T physMemUsedByMe = pmc.WorkingSetSize;

    // VRAM profiling  
    /*DXGI_QUERY_VIDEO_MEMORY_INFO vramInfo = {};
    ComPtr<IDXGIAdapter3> adapter;
    HRESULT hr = pDevice->Get()->QueryInterface(IID_PPV_ARGS(&adapter));
    if (SUCCEEDED(hr))
    {
        adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vramInfo);
    }
    SIZE_T vramUsed = vramInfo.CurrentUsage / (1024 * 1024);

    SIZE_T total = (physMemUsedByMe / (1024 * 1024)) + (virtualMemUsedByMe / (1024 * 1024));*/

    // Update window title with memory usage  
    std::string memoryInfo = "   |   [PhysMemUsed: " + std::to_string(physMemUsedByMe / (1024 * 1024)) + "MB";
    memoryInfo += "VirtMemUsed: " + std::to_string(virtualMemUsedByMe / (1024 * 1024)) + "MB";
    //memoryInfo += " TotalMemUsed : " + std::to_string(total) + "MB]";
    //memoryInfo += " | " + "[VRAMUsed:" + std::to_string(vramUsed) + "MB]";

    if(isRefreshTick)
    {
        static int frame;
        static float fps;
        if (++frame % 60 == 0)
        {
            fps = 1.0f / pTime->DeltaTime();
        }
        pWindow->SetName(StringToWString(title + "   |   [DeltaTime: " + std::to_string(pTime->DeltaTime()) + "ms]" + memoryInfo + "  |  " + "[Performance: " + std::to_string(static_cast<int>(fps)) + "fps]").c_str());
    }
    else
    {
        pWindow->SetName(StringToWString(title + "   |   [DeltaTime: " + std::to_string(pTime->DeltaTime()) + "ms]" + memoryInfo).c_str());
    }
}

void glib::BeginRecordPerformance()
{
    isRefreshTick = false;
    QueryPerformanceFrequency(&freq);

    QueryPerformanceCounter(&start);
}

void glib::EndRecordPerformance()
{
    QueryPerformanceCounter(&end);
    isRefreshTick = true;
}

void glib::SetMaxFPS(int fps)
{
    MaxFPS = fps;
}

int glib::GetMaxFPS()
{
    return MaxFPS;
}
