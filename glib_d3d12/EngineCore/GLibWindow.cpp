#include <GLibWindow.h>
#include <GLibLogger.h>
#include <GLib.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            glib::Logger::InfoLog("Escape�L�[�������ꂽ���߁A�A�v���P�[�V�������I�����܂��B");
            return 0;
        }
        break;

    case WM_PAINT:
        ValidateRect(hwnd, nullptr);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

glib::GLibWindow::GLibWindow()
{
    m_WindowTitle = L"GLib_D3D12";
    m_DefaultWindowTitle = m_WindowTitle;
    m_ClientWidth = 1280;
    m_ClientHeight = 720;
    m_ClientPosX = (GetSystemMetrics(SM_CXSCREEN) - m_ClientWidth) / 2;
    m_ClientPosY = (GetSystemMetrics(SM_CYSCREEN) - m_ClientHeight) / 2;
    m_Aspect = static_cast<float>(m_ClientWidth) / static_cast<float>(m_ClientHeight);
    m_WindowStyle = WS_OVERLAPPEDWINDOW;
    m_HWnd = nullptr;
    m_Rect = {};
}

glib::GLibWindow::~GLibWindow()
{
}

void glib::GLibWindow::Finalize(const LPCWSTR& wndName, int width, int height)
{
    m_WindowTitle = wndName;
    m_ClientWidth = width;
    m_ClientHeight = height;

    // �E�B���h�E�N���X�̓o�^
    {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(0);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
        wc.lpszClassName = L"EngineWindowClass";
        RegisterClassEx(&wc);
        glib::Logger::FormatDebugLog("Window class registered: %s", glib::WStringToString(wc.lpszClassName).c_str());
    }

    // �\���ʒu�A�E�B���h�E�̑傫������
    {
        m_Rect = { 0, 0, m_ClientWidth, m_ClientHeight };
        AdjustWindowRect(&m_Rect, m_WindowStyle, FALSE);
        int windowPosX = m_ClientPosX + m_Rect.left;
        int windowPosY = m_ClientPosY + m_Rect.top;
        int windowWidth = m_Rect.right - m_Rect.left;
        int windowHeight = m_Rect.bottom - m_Rect.top;
        // �E�B���h�E�����
        m_HWnd = CreateWindowEx(
            0,
            L"EngineWindowClass",
            m_WindowTitle,
            m_WindowStyle,
            windowPosX, windowPosY, windowWidth, windowHeight,
            nullptr, nullptr, GetModuleHandle(nullptr), nullptr
        );
        glib::Logger::FormatDebugLog("Window created: %s at (%d, %d) with size (%d, %d)", glib::WStringToString(m_WindowTitle).c_str(), windowPosX, windowPosY, windowWidth, windowHeight);
    }

    if (m_HWnd == nullptr)
    {
        MessageBox(nullptr, L"Failed to create window", L"Error", MB_ICONERROR);
        glib::Logger::ErrorLog("Failed to create window.");
        return;
    }

    ShowWindow(m_HWnd, SW_SHOW);
    glib::Logger::DebugLog("Window shown successfully.");
}

void glib::GLibWindow::SetName(const LPCWSTR& wndName, bool isOutputLog)
{
    m_WindowTitle = wndName;
    if (m_HWnd)
    {
        SetWindowText(m_HWnd, m_WindowTitle);
        if (isOutputLog)
            Logger::FormatDebugLog("�E�B���h�E�^�C�g����ύX: %s", glib::WStringToString(m_WindowTitle).c_str());
    }
    else
    {
        if (isOutputLog)
            Logger::WarningLog("�E�B���h�E�^�C�g����ύX�ł��܂���ł����i�E�B���h�E�����݂��Ȃ����߁j�B");
    }
}

void glib::GLibWindow::SetPos(int x, int y)
{
    m_ClientPosX = x;
    m_ClientPosY = y;

    if (m_HWnd)
    {
        SetWindowPos(m_HWnd, nullptr, m_ClientPosX, m_ClientPosY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        Logger::FormatDebugLog("�E�B���h�E�ʒu��ύX: (%d, %d)", m_ClientPosX, m_ClientPosY);
    }
    else
    {
        Logger::WarningLog("�E�B���h�E�ʒu��ύX�ł��܂���ł����i�E�B���h�E�����݂��Ȃ����߁j�B");
    }
}

void glib::GLibWindow::SetAspect(float aspect)
{
    m_Aspect = aspect;

    if (m_HWnd)
    {
        RECT rect;
        GetClientRect(m_HWnd, &rect);
        int newWidth = static_cast<int>(rect.bottom * m_Aspect);
        SetWindowPos(m_HWnd, nullptr, 0, 0, newWidth, rect.bottom, SWP_NOMOVE | SWP_NOZORDER);
        Logger::FormatDebugLog("�A�X�y�N�g���ݒ�: %.2f�i�V������: %d�j", m_Aspect, newWidth);
    }
    else
    {
        Logger::WarningLog("�A�X�y�N�g���ݒ�ł��܂���ł����i�E�B���h�E�����݂��Ȃ����߁j�B");
    }
}

void glib::GLibWindow::SetStyle(DWORD style)
{
    m_WindowStyle = style;

    if (m_HWnd)
    {
        SetWindowLong(m_HWnd, GWL_STYLE, m_WindowStyle);
        SetWindowPos(m_HWnd, nullptr, 0, 0, m_ClientWidth, m_ClientHeight, SWP_NOMOVE | SWP_NOZORDER);
        Logger::FormatDebugLog("�E�B���h�E�X�^�C����ݒ�: 0x%08X", m_WindowStyle);
    }
    else
    {
        Logger::WarningLog("�E�B���h�E�X�^�C����ݒ�ł��܂���ł����i�E�B���h�E�����݂��Ȃ����߁j�B");
    }
}

void glib::GLibWindow::SetClientWidth(int width)
{
    m_ClientWidth = width;
    if (m_HWnd)
    {
        RECT rect;
        GetClientRect(m_HWnd, &rect);
        int newHeight = static_cast<int>(rect.right / m_Aspect);
        SetWindowPos(m_HWnd, nullptr, 0, 0, m_ClientWidth, newHeight, SWP_NOMOVE | SWP_NOZORDER);
        Logger::FormatDebugLog("�N���C�A���g����ݒ�: %d�i�V��������: %d�j", m_ClientWidth, newHeight);
    }
    else
    {
        Logger::WarningLog("�N���C�A���g����ݒ�ł��܂���ł����i�E�B���h�E�����݂��Ȃ����߁j�B");
    }
}

void glib::GLibWindow::SetClientHeight(int height)
{
    m_ClientHeight = height;
    if (m_HWnd)
    {
        RECT rect;
        GetClientRect(m_HWnd, &rect);
        int newWidth = static_cast<int>(rect.bottom * m_Aspect);
        SetWindowPos(m_HWnd, nullptr, 0, 0, newWidth, m_ClientHeight, SWP_NOMOVE | SWP_NOZORDER);
        Logger::FormatDebugLog("�N���C�A���g������ݒ�: %d�i�V������: %d�j", m_ClientHeight, newWidth);
    }
    else
    {
        Logger::WarningLog("�N���C�A���g������ݒ�ł��܂���ł����i�E�B���h�E�����݂��Ȃ����߁j�B");
    }
}