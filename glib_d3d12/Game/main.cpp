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

#define FIX_FRAME_RATE  // �t���[�����[�g���Œ�ɂ���

INT WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ INT)
{
    // GLib�̏�����
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
        HDC hdc = GetDC(glib::GetWindow()->GetHWnd());	// �f�o�C�X�R���e�L�X�g�̎擾
        float refreshRate = (float)GetDeviceCaps(hdc, VREFRESH);	// ���t���b�V�����[�g�̎擾
        refreshTime = 1.0f / refreshRate;
        ReleaseDC(glib::GetWindow()->GetHWnd(), hdc);	// �f�o�C�X�R���e�L�X�g�̉��
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
        // �X�V�񐔂��ő�fps�ɍ��킹�ė}�����肷��
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
        // GLib�̎��ԍX�V
        glib::RefreshDeltaTime();

        // �������x�v���J�n
        glib::BeginRecordPerformance();

        // GLib�̃����_�����O�J�n
        glib::BeginRender();

        if (not glib::Message::ProcessMessage())
            break;

        // GLib�̃����_�����O�I��
        glib::EndRender();

        // �������x�v���I��
        glib::EndRecordPerformance();

        // �v���t�@�C�����O����
        glib::RunProfile();
    }

    glib::Logger::InfoLog("�A�v���P�[�V�������I�����܂��B");
    glib::Release();

#ifdef _DEBUG
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtDumpMemoryLeaks();
#endif
    return 0;
}