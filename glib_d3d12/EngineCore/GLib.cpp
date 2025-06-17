

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
    glib::Logger::CriticalLog("GLib�̏��������J�n���܂��B");

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
        glib::Logger::ErrorLog("�E�B���h�E�̍쐬�Ɏ��s���܂����B");
        return false;
    }
    else
    {
        glib::Logger::DebugLog("�E�B���h�E�̍쐬�ɐ������܂���: " + glib::StringUtil::WStringToString(pWindow->GetName()));
    }

    pD3D12 = new GLibD3D12Wrapper;

    // Initialize the time management
    pTime->SetLevelLoaded();
    glib::Logger::DebugLog("���ԊǗ��V�X�e���̏������ɐ������܂����B");


    glib::Logger::DebugLog("GLib�̏�����������Ɋ������܂����B");
    glib::Logger::CriticalLog("GLib�̏��������I�����܂��B");
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
    return true;
}

void glib::BeginRender(const GLIB_PIPELINE_TYPE& usePipelineType)
{
    // ���C�����[�v  
    {
        static float radian = 0.0f;
        radian += XMConvertToRadians(10.0f) * pTime->DeltaTime();
        pD3D12->SetClearColor(XMFLOAT4(sinf(radian), 1.0f, cosf(radian), 1.0f));
    }

    // �`���t�J�n
    pD3D12->BeginRender();
}

void glib::EndRender(const GLIB_PIPELINE_TYPE& usePipelineType)
{
    pD3D12->EndRender();
}

void glib::Release()
{
    glib::Logger::CriticalLog("---=====================[GLIB]=====================---");
    glib::Logger::CriticalLog("GLib�̉���������J�n���܂��B");

    SafeDelete(pTime);
    glib::Logger::DebugLog("���ԊǗ��V�X�e���̉���ɐ������܂����B");

    SafeDelete(pD3D12);
    glib::Logger::DebugLog("Direct3D 12�̉���ɐ������܂����B");

    // Release the window
    if (pWindow->GetHWnd())
    {
        DestroyWindow(pWindow->GetHWnd());
        SafeDelete(pWindow);
        glib::Logger::DebugLog("�E�B���h�E�̉���ɐ������܂����B");
    }

    // Log release message
    glib::Logger::DebugLog("GLib�̉��������Ɋ������܂����B");
    glib::Logger::CriticalLog("GLib�̉���������I�����܂��B");
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
    //��x�ڂ̌Ăяo���͕����񐔂�m�邽��
    auto result = MultiByteToWideChar(CP_UTF8,
        0,
        str.c_str(),//���͕�����
        (int)str.length(),
        nullptr,
        0);
    assert(result >= 0);
    ret.resize(result);//�m�ۂ���
    //��x�ڂ̌Ăяo���͕ϊ�
    result = MultiByteToWideChar(CP_UTF8,
        0,
        str.c_str(),//���͕�����
        (int)str.length(),
        ret.data(),
        (int)ret.size());
    return ret;
}

std::string glib::WStringToString(const std::wstring& wstr)
{
    std::string ret;
    //��x�ڂ̌Ăяo���͕����񐔂�m�邽��
    auto result = WideCharToMultiByte(
        CP_ACP,
        0,
        wstr.c_str(),//���͕�����
        (int)wstr.length(),
        nullptr,
        0,
        nullptr,
        nullptr);
    assert(result >= 0);
    ret.resize(result);//�m�ۂ���
    //��x�ڂ̌Ăяo���͕ϊ�
    result = WideCharToMultiByte(
        CP_ACP,
        0,
        wstr.c_str(),//���͕�����
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
