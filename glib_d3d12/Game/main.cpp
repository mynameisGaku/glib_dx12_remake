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

#define FIX_FRAME_RATE  // フレームレートを固定にする

INT WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ INT)
{
    // GLibの初期化
    if (not glib::Init())
    {
        return -1;
    }

    glib::SetMaxFPS(60);

#ifdef FIX_FRAME_RATE
    float refreshTime;
    float freq;
    LONGLONG lastTime;
    if (glib::GetMaxFPS() <= 0.0f)
    {
        HDC hdc = GetDC(glib::GetWindow()->GetHWnd());	// デバイスコンテキストの取得
        float refreshRate = (float)GetDeviceCaps(hdc, VREFRESH);	// リフレッシュレートの取得
        refreshTime = 1.0f / refreshRate;
        ReleaseDC(glib::GetWindow()->GetHWnd(), hdc);	// デバイスコンテキストの解放
    }
    else
    {
        int d = glib::GetMaxFPS();
        refreshTime = 1.0f / d;
    }
    {
        LARGE_INTEGER freqL;
        QueryPerformanceFrequency(&freqL);
        freq = (float)freqL.QuadPart;
        LARGE_INTEGER current;
        QueryPerformanceCounter(&current);
        lastTime = current.QuadPart;
    }
#endif

    while (true)
    {
        // 更新回数を最大fpsに合わせて抑えたりする
#ifdef FIX_FRAME_RATE
        while (true)
        {
            LARGE_INTEGER current;
            QueryPerformanceCounter(&current);
            float dt = static_cast<float>(current.QuadPart - lastTime) / freq;
            if (dt >= refreshTime)
            {
                lastTime = current.QuadPart;
                break;
            }
        }
#endif
        // GLibの時間更新
        glib::RefreshDeltaTime();

        // 処理速度計測開始
        glib::BeginRecordPerformance();

        // GLibのレンダリング開始
        glib::BeginRender();

        if (not glib::Message::ProcessMessage())
            break;

        // GLibのレンダリング終了
        glib::EndRender();

        // 処理速度計測終了
        glib::EndRecordPerformance();

        // プロファイリングする
        glib::RunProfile();
    }

    glib::Logger::InfoLog("アプリケーションを終了します。");
    glib::Release();

#ifdef _DEBUG
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtDumpMemoryLeaks();
#endif
    return 0;
}