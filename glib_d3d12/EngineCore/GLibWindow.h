#pragma once
#include <Windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace glib
{
    class GLibWindow
    {
    public:

        GLibWindow();
        ~GLibWindow();

        void Finalize(const LPCWSTR& wndName, int width, int height);

        void SetName(const LPCWSTR& wndName);
        void SetPos(int x, int y);
        void SetAspect(float aspect);
        void SetStyle(DWORD style);
        void SetClientWidth(int width);
        void SetClientHeight(int height);

        LPCWSTR GetName() const
        {
            return m_WindowTitle;
        }
        LPCWSTR GetDefaultName() const
        {
            return m_DefaultWindowTitle;
        }
        HWND GetHWnd() const
        {
            return m_HWnd;
        }
        RECT GetRect() const
        {
            return m_Rect;
        }
        int GetClientWidth() const
        {
            return m_ClientWidth;
        }
        int GetClientHeight() const
        {
            return m_ClientHeight;
        }
        int GetClientPosX() const
        {
            return m_ClientPosX;
        }
        int GetClientPosY() const
        {
            return m_ClientPosY;
        }
        float GetAspect() const
        {
            return m_Aspect;
        }
        DWORD GetStyle() const
        {
            return m_WindowStyle;
        }
        
    private:

        LPCWSTR m_DefaultWindowTitle;
        LPCWSTR m_WindowTitle;
        int m_ClientWidth;
        int m_ClientHeight;
        int m_ClientPosX;
        int m_ClientPosY;
        float m_Aspect;

        DWORD m_WindowStyle;
        HWND m_HWnd;
        RECT m_Rect;
    };
}