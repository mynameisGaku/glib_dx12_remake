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
    case WM_PAINT:
        ValidateRect(hwnd, nullptr);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

glib::GLibWindow::GLibWindow()
{
    m_WindowTitle = L"GLib_D3D12";
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

    // ウィンドウクラスの登録
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

    // 表示位置、ウィンドウの大きさ調整
    {
        m_Rect = { 0, 0, m_ClientWidth, m_ClientHeight };
        AdjustWindowRect(&m_Rect, m_WindowStyle, FALSE);
        int windowPosX = m_ClientPosX + m_Rect.left;
        int windowPosY = m_ClientPosY + m_Rect.top;
        int windowWidth = m_Rect.right - m_Rect.left;
        int windowHeight = m_Rect.bottom - m_Rect.top;
        // ウィンドウを作る
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
        return;
    }

    ShowWindow(m_HWnd, SW_SHOW);
    glib::Logger::DebugLog("Window shown successfully.");
}

void glib::GLibWindow::SetName(const LPCWSTR& wndName)
{
    m_WindowTitle = wndName;
    if (m_HWnd)
    {
        SetWindowText(m_HWnd, m_WindowTitle);
    }
}

void glib::GLibWindow::SetPos(int x, int y)
{
    m_ClientPosX = x;
    m_ClientPosY = y;

    if (m_HWnd)
    {
        SetWindowPos(m_HWnd, nullptr, m_ClientPosX, m_ClientPosY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
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
    }
}

void glib::GLibWindow::SetStyle(DWORD style)
{
    m_WindowStyle = style;

    if (m_HWnd)
    {
        SetWindowLong(m_HWnd, GWL_STYLE, m_WindowStyle);
        SetWindowPos(m_HWnd, nullptr, 0, 0, m_ClientWidth, m_ClientHeight, SWP_NOMOVE | SWP_NOZORDER);
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
    }
}
