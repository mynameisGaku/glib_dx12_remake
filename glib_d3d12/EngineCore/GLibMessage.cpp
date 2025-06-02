#include <GLibMessage.h>
#include <Windows.h>

bool glib::Message::ProcessMessage()
{
    MSG msg = { 0 };
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}
