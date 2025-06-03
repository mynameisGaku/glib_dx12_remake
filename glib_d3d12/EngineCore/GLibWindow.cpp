#include "GLibWindow.h"

/* static memebers init*/
LPCWSTR glib::Window::m_WindowTitle = L"GLib_D3D12";
int glib::Window::ClientWidth = 1280;
int glib::Window::ClientHeight = 720;
int glib::Window::ClientPosX = (GetSystemMetrics(SM_CXSCREEN) - ClientWidth) / 2;
int glib::Window::ClientPosY = (GetSystemMetrics(SM_CYSCREEN) - ClientHeight) / 2;
float glib::Window::Aspect = static_cast<float>(ClientWidth) / static_cast<float>(ClientHeight);
DWORD glib::Window::WindowStyle = WS_OVERLAPPEDWINDOW;
HWND glib::Window::m_HWnd = nullptr;
RECT glib::Window::m_Rect{};

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

void glib::Window::Finalize(const LPCWSTR& wndName, int width, int height)
{
    m_WindowTitle = wndName;
    ClientWidth = width;
    ClientHeight = height;

    // ウィンドウクラスの登録
    {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(0);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.lpszClassName = L"EngineWindowClass";
        RegisterClassEx(&wc);
    }

    // 表示位置、ウィンドウの大きさ調整
    {
        m_Rect = { 0, 0, ClientWidth, ClientHeight };
        AdjustWindowRect(&m_Rect, WindowStyle, FALSE);
        int windowPosX = ClientPosX + m_Rect.left;
        int windowPosY = ClientPosY + m_Rect.top;
        int windowWidth = m_Rect.right - m_Rect.left;
        int windowHeight = m_Rect.bottom - m_Rect.top;
        // ウィンドウを作る
        m_HWnd = CreateWindowEx(
            0,
            L"EngineWindowClass",
            m_WindowTitle,
            WindowStyle,
            windowPosX, windowPosY, windowWidth, windowHeight,
            nullptr, nullptr, GetModuleHandle(nullptr), nullptr
        );
    }

    if (m_HWnd == nullptr)
    {
        MessageBox(nullptr, L"Failed to create window", L"Error", MB_ICONERROR);
        return;
    }

    ShowWindow(m_HWnd, SW_SHOW);
}

void glib::Window::SetName(const LPCWSTR& wndName)
{
    m_WindowTitle = wndName;
    if (m_HWnd)
    {
        SetWindowText(m_HWnd, m_WindowTitle);
    }
}

void glib::Window::SetPos(int x, int y)
{
    ClientPosX = x;
    ClientPosY = y;

    if (m_HWnd)
    {
        SetWindowPos(m_HWnd, nullptr, ClientPosX, ClientPosY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void glib::Window::SetAspect(float aspect)
{
    Aspect = aspect;

    if (m_HWnd)
    {
        RECT rect;
        GetClientRect(m_HWnd, &rect);
        int newWidth = static_cast<int>(rect.bottom * Aspect);
        SetWindowPos(m_HWnd, nullptr, 0, 0, newWidth, rect.bottom, SWP_NOMOVE | SWP_NOZORDER);
    }
}

void glib::Window::SetStyle(DWORD style)
{
    WindowStyle = style;

    if (m_HWnd)
    {
        SetWindowLong(m_HWnd, GWL_STYLE, WindowStyle);
        SetWindowPos(m_HWnd, nullptr, 0, 0, ClientWidth, ClientHeight, SWP_NOMOVE | SWP_NOZORDER);
    }
}