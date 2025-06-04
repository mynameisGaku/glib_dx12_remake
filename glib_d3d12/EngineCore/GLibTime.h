#pragma once
#include <chrono>
#include <deque>


namespace glib
{

    /// <summary>
    /// Unity��Time�N���X�Ɋ�Â������ԊǗ��N���X�B
    /// �Q�[�����ԁE�Œ莞�ԁE�����ԁE�X�P�[������E�e��⏕�l�Ȃǂ�񋟁B
    /// </summary>
    class GLibTime
    {
    public:

        GLibTime();

        /// <summary>
        /// ���ԉ��Z���@
        /// </summary>
        enum class AdditionMethod {
            None = 0,	// ���Z���Ȃ�
            Usual,		// ���Z����(���Ԍo�ߗ����܂܂Ȃ�)
            Rate		// ���Z����(���Ԍo�ߗ����܂�)
        };

        /// <summary>���t���[���X�V�i�ʏ�Update�p�j</summary>
        void Update();

        /// <summary>FixedUpdate�J�n���ɌĂяo��</summary>
        void BeginFixedUpdate();

        /// <summary>FixedUpdate�I�����ɌĂяo��</summary>
        void EndFixedUpdate();

        /// <summary>�V�[���؂�ւ����ɌĂяo��</summary>
        void SetLevelLoaded();

        /// <summary>DeltaTime�itimeScale�̉e������j</summary>
        float DeltaTime();

        /// <summary>UnscaledDeltaTime�itimeScale�̉e���Ȃ��j</summary>
        float UnscaledDeltaTime();

        /// <summary>FixedDeltaTime�itimeScale�̉e������j</summary>
        float FixedDeltaTime();

        /// <summary>FixedUnscaledDeltaTime�i��ɌŒ�l�j</summary>
        float FixedUnscaledDeltaTime();

        /// <summary>�N������̗݌v���ԁitimeScale�̉e������j</summary>
        float TotalTime();

        /// <summary>�N������̗݌v���� int�ŁitimeScale�̉e������j</summary>
        int   TotalTimeInt();

        /// <summary>�N������̗݌v���ԁitimeScale�̉e���Ȃ��j</summary>
        float UnscaledTime();

        /// <summary>�N������̃��A�����ԁiUnscaledTime�Ɠ����j</summary>
        float RealtimeSinceStartup();

        /// <summary>�V�[���ǂݍ��݂���̌o�ߎ��ԁitimeScale�̉e������j</summary>
        float TimeSinceLevelLoad();

        /// <summary>���݂̃t���[����</summary>
        int FrameCount();

        /// <summary>FixedUpdate�����ǂ���</summary>
        bool InFixedTimeStep();

        /// <summary>���炩�ɕ�Ԃ��ꂽDeltaTime</summary>
        float SmoothDeltaTime();

        /// <summary>DeltaTime�̏���l</summary>
        float MaximumDeltaTime();

        /// <summary>���Ԕ{����ݒ�</summary>
        void SetTimeScale(float scale);

        /// <summary>���݂̎��Ԕ{�����擾</summary>
        float GetTimeScale();

        /// <summary>FixedDeltaTime��ݒ�</summary>
        void SetFixedDeltaTime(float fixedDelta);

        /// <summary>DeltaTime�̍ő�l��ݒ�</summary>
        void SetMaximumDeltaTime(float maxDelta);

        /// <summary>
        /// �q�b�g�X�g�b�v��ݒ肷��
        /// </summary>
        /// <param name="frame">�t���[����</param>
        void SetHitStop(int frame);

        /// <summary>
        /// ���݂̃q�b�g�X�g�b�v�̃t���[�������擾����
        /// </summary>
        int HitStop();

        /// <summary>
        /// �q�b�g�X�g�b�v����
        /// </summary>
        /// <returns>�q�b�g�X�g�b�v���Ȃ�true</returns>
        bool IsHitStop();

        /// <summary>
        /// �ő�FPS��ݒ肷��
        /// </summary>
        void SetMaxFPS(int fps);

        //==================================================
        // ���ϊ�

        // �t���[����b�ɕϊ�
        template<typename Ty = float>
        inline float FrameToSec(Ty frame) { return static_cast<float>(frame) / 60.0f; }

        // �t���[���𕪂ɕϊ�
        template<typename Ty = float>
        inline float FrameToMin(Ty frame) { return static_cast<float>(frame) / 60.0f * 60.0f; }

        // �t���[�������Ԃɕϊ�
        template<typename Ty = float>
        inline float FrameToHour(Ty frame) { return static_cast<float>(frame) / 3600.0f * 60.0f; }

        // �b���t���[���ɕϊ�
        template<typename Ty = float>
        inline float SecToFrame(Ty sec) { return static_cast<float>(sec) * 60.0f; }

        // �����t���[���ɕϊ�
        template<typename Ty = float>
        inline float MinToFrame(Ty min) { return static_cast<float>(min) * 60.0f * 60.0f; }

        // ���Ԃ��t���[���ɕϊ�
        template<typename Ty = float>
        inline float HourToFrame(Ty hour) { return static_cast<float>(hour) * 60.0f * 60.0f * 60.0f; }

    private:

        void initialize();
        bool m_Initialized;

        std::chrono::high_resolution_clock::time_point m_StartTime;
        std::chrono::high_resolution_clock::time_point m_LastTime;
        std::chrono::high_resolution_clock::time_point m_LevelLoadTime;

        float m_DeltaTime;
        float m_TimeScale;
        float m_UnscaledDeltaTime;
        float m_SmoothDeltaTime;
        float m_FixedDeltaTime;
        float m_MaximumDeltaTime;
        int m_FrameCount;
        int m_HitStop;
        int m_MaxFps;
        bool m_InFixedTimeStep;
        std::deque<float> deltaHistory;
    };
}