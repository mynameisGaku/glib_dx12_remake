#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifdef _DEBUG
#define DBG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__);
#else
#define DBG_NEW new
#endif

#include <GLib.h>
#include <GLibMessage.h>
#include <GLibTime.h>

INT WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ INT)
{
    // GLib�̏�����
    if (not glib::Init())
    {
        return -1;
    }

    while (true)
    {
        // �������x�v���J�n
        glib::BeginRecordPerformance();

        // GLib�̃����_�����O�J�n
        glib::BeginRender();

        // GLib�̎��ԍX�V
        glib::RefreshDeltaTime();

        if (not glib::Message::ProcessMessage())
            break;

        // GLib�̃����_�����O�I��
        glib::EndRender();

        // �������x�v���I��
        glib::EndRecordPerformance();

        // �v���t�@�C�����O����
        glib::RunProfile();
    }

    glib::Release();

#ifdef _DEBUG
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtDumpMemoryLeaks();
#endif
    return 0;
}