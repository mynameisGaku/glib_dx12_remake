#include <GLib.h>
#include <GLibDevice.h>
#include <GLibDebug.h>
#include <GLibCommandAllocator.h>
#include <GLibCommandQueue.h>
#include <GLibGraphicsCommandList.h>
#include <GLibFence.h>
#include <GLibStringUtil.h>

/* pragma link */
#pragma comment(lib, "d3d12.lib")

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
    */

    // Initialize the device
    glib::GLibDevice::GetInstance().Initialize(D3D_FEATURE_LEVEL_12_0);


    // Initialize the command allocator
    glib::GLibCommandAllocator::GetInstance().Initialize(glib::GLibDevice::GetInstance().Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);


    // Initialize the command list
    glib::GLibGraphicsCommandList::GetInstance().Initialize(glib::GLibDevice::GetInstance().Get(), glib::GLibCommandAllocator::GetInstance().Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);


    // Initialize the command queue
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    glib::GLibCommandQueue::GetInstance().Initialize(glib::GLibDevice::GetInstance().Get(), desc);


    // Initialize the fence
    glib::GLibFence::GetInstance().Initialize(glib::GLibDevice::GetInstance().Get(), D3D12_FENCE_FLAG_NONE);


    glib::Logger::DebugLog("GLib initialized successfully.");
    return true;
}

void glib::Release()
{

    // D3D12 Release
    /*
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
    glib::GLibFence::GetInstance().Release();
    glib::GLibCommandQueue::GetInstance().Release();
    glib::GLibGraphicsCommandList::GetInstance().Release();
    glib::GLibCommandAllocator::GetInstance().Release();
    glib::GLibDevice::GetInstance().Release();

    // Release the window
    if (glib::Window::m_HWnd)
    {
        DestroyWindow(glib::Window::m_HWnd);
        glib::Window::m_HWnd = nullptr;
    }

    // Log release message
    glib::Logger::DebugLog("GLib released successfully.");
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
