

// vendor
#include <Vendor/magic_enum/magic_enum.hpp>

#include <GLib.h>
#include <GLibDebug.h>
#include <GLibStringUtil.h>
#include <GLibTime.h>
#include <GLibMemory.h>
#include <GLibWindow.h>
#include <GLibBinaryLoader.h>
#include <GLibD3D12Wrapper.h>

using namespace DirectX;

namespace glib
{
    /* constants */
    const int SHADER_MAX = 10;

    // config
    int MaxFPS = 60;
    int BackBufferCountMax = 2;

    /* device interfaces */
    glib::GLibTime* pTime;
    glib::GLibWindow* pWindow;
    glib::GLibD3D12Wrapper* pD3D12;


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
    glib::Logger::CriticalLog("GLibの初期化を開始します。");

    {
        pWindow = new GLibWindow;
        pTime = new GLibTime;

        ViewPort = { 0.0f, 0.0f, static_cast<float>(pWindow->GetClientWidth()), static_cast<float>(pWindow->GetClientHeight()), 0.0f, 1.0f };
        ScissorRect = { 0, 0, pWindow->GetClientWidth(), pWindow->GetClientHeight() };
    }

    // Initialize the window
    pWindow->Finalize(pWindow->GetName(), pWindow->GetClientWidth(), pWindow->GetClientHeight());

    if (!pWindow->GetHWnd())
    {
        glib::Logger::ErrorLog("ウィンドウの作成に失敗しました。");
        return false;
    }
    else
    {
        glib::Logger::DebugLog("ウィンドウの作成に成功しました: " + glib::StringUtil::WStringToString(pWindow->GetName()));
    }

    pD3D12 = new GLibD3D12Wrapper;

    // Initialize the time management
    pTime->SetLevelLoaded();
    glib::Logger::DebugLog("時間管理システムの初期化に成功しました。");


    glib::Logger::DebugLog("GLibの初期化が正常に完了しました。");
    glib::Logger::CriticalLog("GLibの初期化を終了します。");
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
    return true;
}

void glib::BeginRender(const GLIB_PIPELINE_TYPE& usePipelineType)
{
    // メインループ  
    {
        static float radian = 0.0f;
        radian += XMConvertToRadians(10.0f) * pTime->DeltaTime();
        pD3D12->SetClearColor(XMFLOAT4(sinf(radian), 1.0f, cosf(radian), 1.0f));
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
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
    glib::Logger::CriticalLog("GLibの解放処理を開始します。");

    SafeDelete(pTime);
    glib::Logger::DebugLog("時間管理システムの解放に成功しました。");

    SafeDelete(pD3D12);
    glib::Logger::DebugLog("Direct3D 12の解放に成功しました。");

    // Release the window
    if (pWindow->GetHWnd())
    {
        DestroyWindow(pWindow->GetHWnd());
        SafeDelete(pWindow);
        glib::Logger::DebugLog("ウィンドウの解放に成功しました。");
    }

    // Log release message
    glib::Logger::DebugLog("GLibの解放が正常に完了しました。");
    glib::Logger::CriticalLog("GLibの解放処理を終了します。");
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
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

D3D12_VIEWPORT glib::GetViewport()
{
    return ViewPort;
}

D3D12_RECT glib::GetRect()
{
    return ScissorRect;
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
    pD3D12->RunProfile();
}

void glib::BeginRecordPerformance()
{
    pD3D12->BeginRecordPerformance();
}

void glib::EndRecordPerformance()
{
    pD3D12->EndRecordPerformance();
}

void glib::SetMaxFPS(int fps)
{
    MaxFPS = fps;
}

int glib::GetMaxFPS()
{
    return MaxFPS;
}
