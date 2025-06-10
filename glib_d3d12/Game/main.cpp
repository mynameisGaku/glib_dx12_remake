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
    // GLibの初期化
    if (not glib::Init())
    {
        return -1;
    }

    while (true)
    {
        // 処理速度計測開始
        glib::BeginRecordPerformance();

        // GLibのレンダリング開始
        glib::BeginRender();

        // GLibの時間更新
        glib::RefreshDeltaTime();

        if (not glib::Message::ProcessMessage())
            break;

        // GLibのレンダリング終了
        glib::EndRender();

        // 処理速度計測終了
        glib::EndRecordPerformance();

        // プロファイリングする
        glib::RunProfile();
    }

    glib::Release();

#ifdef _DEBUG
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtDumpMemoryLeaks();
#endif
    return 0;
}