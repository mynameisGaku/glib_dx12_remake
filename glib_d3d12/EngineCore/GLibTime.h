#pragma once
#include <chrono>


namespace glib
{

    /// <summary>
    /// Unity��Time�N���X�Ɋ�Â������ԊǗ��N���X�B
    /// �Q�[�����ԁE�Œ莞�ԁE�����ԁE�X�P�[������E�e��⏕�l�Ȃǂ�񋟁B
    /// </summary>
    class GLibTime
    {
    public:

        /// <summary>
        /// ���ԉ��Z���@
        /// </summary>
        enum class AdditionMethod {
            None = 0,	// ���Z���Ȃ�
            Usual,		// ���Z����(���Ԍo�ߗ����܂܂Ȃ�)
            Rate		// ���Z����(���Ԍo�ߗ����܂�)
        };

        /// <summary>���t���[���X�V�i�ʏ�Update�p�j</summary>
        static void Update();

        /// <summary>FixedUpdate�J�n���ɌĂяo��</summary>
        static void BeginFixedUpdate();

        /// <summary>FixedUpdate�I�����ɌĂяo��</summary>
        static void EndFixedUpdate();

        /// <summary>�V�[���؂�ւ����ɌĂяo��</summary>
        static void SetLevelLoaded();

        /// <summary>DeltaTime�itimeScale�̉e������j</summary>
        static float DeltaTime();

        /// <summary>UnscaledDeltaTime�itimeScale�̉e���Ȃ��j</summary>
        static float UnscaledDeltaTime();

        /// <summary>FixedDeltaTime�itimeScale�̉e������j</summary>
        static float FixedDeltaTime();

        /// <summary>FixedUnscaledDeltaTime�i��ɌŒ�l�j</summary>
        static float FixedUnscaledDeltaTime();

        /// <summary>�N������̗݌v���ԁitimeScale�̉e������j</summary>
        static float TotalTime();

        /// <summary>�N������̗݌v���� int�ŁitimeScale�̉e������j</summary>
        static int   TotalTimeInt();

        /// <summary>�N������̗݌v���ԁitimeScale�̉e���Ȃ��j</summary>
        static float UnscaledTime();

        /// <summary>�N������̃��A�����ԁiUnscaledTime�Ɠ����j</summary>
        static float RealtimeSinceStartup();

        /// <summary>�V�[���ǂݍ��݂���̌o�ߎ��ԁitimeScale�̉e������j</summary>
        static float TimeSinceLevelLoad();

        /// <summary>���݂̃t���[����</summary>
        static int FrameCount();

        /// <summary>FixedUpdate�����ǂ���</summary>
        static bool InFixedTimeStep();

        /// <summary>���炩�ɕ�Ԃ��ꂽDeltaTime</summary>
        static float SmoothDeltaTime();

        /// <summary>DeltaTime�̏���l</summary>
        static float MaximumDeltaTime();

        /// <summary>���Ԕ{����ݒ�</summary>
        static void SetTimeScale(float scale);

        /// <summary>���݂̎��Ԕ{�����擾</summary>
        static float GetTimeScale();

        /// <summary>FixedDeltaTime��ݒ�</summary>
        static void SetFixedDeltaTime(float fixedDelta);

        /// <summary>DeltaTime�̍ő�l��ݒ�</summary>
        static void SetMaximumDeltaTime(float maxDelta);

        /// <summary>
        /// �q�b�g�X�g�b�v��ݒ肷��
        /// </summary>
        /// <param name="frame">�t���[����</param>
        static void SetHitStop(int frame);

        /// <summary>
        /// ���݂̃q�b�g�X�g�b�v�̃t���[�������擾����
        /// </summary>
        static int HitStop();

        /// <summary>
        /// �q�b�g�X�g�b�v����
        /// </summary>
        /// <returns>�q�b�g�X�g�b�v���Ȃ�true</returns>
        static bool IsHitStop();

        static GLibTime& GetInstance()
        {
            if (instance == nullptr)
            {
                instance = new GLibTime();
            }
            return (*instance);
        }

        void Destroy()
        {
            if (instance)
            {
                delete instance;
            }
            instance = nullptr;
        }

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

        GLibTime() = default;
        ~GLibTime() = default;

        static GLibTime* instance;

        static void initialize();
        static bool m_Initialized;

        static std::chrono::high_resolution_clock::time_point m_StartTime;
        static std::chrono::high_resolution_clock::time_point m_LastTime;
        static std::chrono::high_resolution_clock::time_point m_LevelLoadTime;

        static float m_DeltaTime;
        static float m_TimeScale;
        static float m_UnscaledDeltaTime;
        static float m_SmoothDeltaTime;
        static float m_FixedDeltaTime;
        static float m_MaximumDeltaTime;
        static int m_FrameCount;
        static int m_HitStop;
        static bool m_InFixedTimeStep;
    };
}