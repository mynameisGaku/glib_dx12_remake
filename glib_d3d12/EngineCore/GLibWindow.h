#pragma once
#include <Windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace glib
{
    class Window
    {
    public:

        static void Finalize(const LPCWSTR& wndName, int width, int height);

        static void SetName(const LPCWSTR& wndName);
        static void SetPos(int x, int y);
        static void SetAspect(float aspect);
        static void SetStyle(DWORD style);

        static LPCWSTR m_WindowTitle;
        static int ClientWidth;
        static int ClientHeight;
        static int ClientPosX;
        static int ClientPosY;
        static float Aspect;

        static DWORD WindowStyle;
        static HWND m_HWnd;
    };
}