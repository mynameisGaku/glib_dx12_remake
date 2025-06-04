#pragma once
#include <chrono>
#include <deque>


namespace glib
{

    /// <summary>
    /// UnityのTimeクラスに基づいた時間管理クラス。
    /// ゲーム時間・固定時間・実時間・スケール制御・各種補助値などを提供。
    /// </summary>
    class GLibTime
    {
    public:

        GLibTime();

        /// <summary>
        /// 時間加算方法
        /// </summary>
        enum class AdditionMethod {
            None = 0,	// 加算しない
            Usual,		// 加算する(時間経過率を含まない)
            Rate		// 加算する(時間経過率を含む)
        };

        /// <summary>毎フレーム更新（通常Update用）</summary>
        void Update();

        /// <summary>FixedUpdate開始時に呼び出す</summary>
        void BeginFixedUpdate();

        /// <summary>FixedUpdate終了時に呼び出す</summary>
        void EndFixedUpdate();

        /// <summary>シーン切り替え時に呼び出す</summary>
        void SetLevelLoaded();

        /// <summary>DeltaTime（timeScaleの影響あり）</summary>
        float DeltaTime();

        /// <summary>UnscaledDeltaTime（timeScaleの影響なし）</summary>
        float UnscaledDeltaTime();

        /// <summary>FixedDeltaTime（timeScaleの影響あり）</summary>
        float FixedDeltaTime();

        /// <summary>FixedUnscaledDeltaTime（常に固定値）</summary>
        float FixedUnscaledDeltaTime();

        /// <summary>起動からの累計時間（timeScaleの影響あり）</summary>
        float TotalTime();

        /// <summary>起動からの累計時間 int版（timeScaleの影響あり）</summary>
        int   TotalTimeInt();

        /// <summary>起動からの累計時間（timeScaleの影響なし）</summary>
        float UnscaledTime();

        /// <summary>起動からのリアル時間（UnscaledTimeと同等）</summary>
        float RealtimeSinceStartup();

        /// <summary>シーン読み込みからの経過時間（timeScaleの影響あり）</summary>
        float TimeSinceLevelLoad();

        /// <summary>現在のフレーム数</summary>
        int FrameCount();

        /// <summary>FixedUpdate中かどうか</summary>
        bool InFixedTimeStep();

        /// <summary>滑らかに補間されたDeltaTime</summary>
        float SmoothDeltaTime();

        /// <summary>DeltaTimeの上限値</summary>
        float MaximumDeltaTime();

        /// <summary>時間倍率を設定</summary>
        void SetTimeScale(float scale);

        /// <summary>現在の時間倍率を取得</summary>
        float GetTimeScale();

        /// <summary>FixedDeltaTimeを設定</summary>
        void SetFixedDeltaTime(float fixedDelta);

        /// <summary>DeltaTimeの最大値を設定</summary>
        void SetMaximumDeltaTime(float maxDelta);

        /// <summary>
        /// ヒットストップを設定する
        /// </summary>
        /// <param name="frame">フレーム数</param>
        void SetHitStop(int frame);

        /// <summary>
        /// 現在のヒットストップのフレーム数を取得する
        /// </summary>
        int HitStop();

        /// <summary>
        /// ヒットストップ中か
        /// </summary>
        /// <returns>ヒットストップ中ならtrue</returns>
        bool IsHitStop();

        /// <summary>
        /// 最大FPSを設定する
        /// </summary>
        void SetMaxFPS(int fps);

        //==================================================
        // ▼変換

        // フレームを秒に変換
        template<typename Ty = float>
        inline float FrameToSec(Ty frame) { return static_cast<float>(frame) / 60.0f; }

        // フレームを分に変換
        template<typename Ty = float>
        inline float FrameToMin(Ty frame) { return static_cast<float>(frame) / 60.0f * 60.0f; }

        // フレームを時間に変換
        template<typename Ty = float>
        inline float FrameToHour(Ty frame) { return static_cast<float>(frame) / 3600.0f * 60.0f; }

        // 秒をフレームに変換
        template<typename Ty = float>
        inline float SecToFrame(Ty sec) { return static_cast<float>(sec) * 60.0f; }

        // 分をフレームに変換
        template<typename Ty = float>
        inline float MinToFrame(Ty min) { return static_cast<float>(min) * 60.0f * 60.0f; }

        // 時間をフレームに変換
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