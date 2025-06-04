#pragma once

#include <GLibWindow.h>
#include <GLibLogger.h>

namespace glib
{
    // 描画パイプラインの種類
    typedef enum
    {
        GLIB_PIPELINE_TYPE_MAIN = 0,                            // 通常の描画を行うパイプライン
        GLIB_PIPELINE_TYPE_SHADOW,                              // 3Dプリミティブの影描画を行うパイプライン
        GLIB_PIPELINE_TYPE_MAIN_NO_SHADOW,                      // 通常の描画を行うが、影の影響を受けないパイプライン
        GLIB_PIPELINE_TYPE_POSTPROCESS,                         // ポストプロセス(画面効果)の描画を行うパイプライン
        GLIB_PIPELINE_TYPE_MAIN_NO_POSTPROCESS_AND_SHADOW,      // 通常の描画を行うが、ポストプロセスと影の影響を受けないパイプライン
        GLIB_PIPELINE_TYPE_UI,                                  // UIの描画を行うパイプライン
        GLIB_PIPELINE_TYPE_COMPUTE,                             // コンピュートシェーダーを使用した計算を行うパイプライン
        GLIB_PIPELINE_TYPE_RAYTRACING,                          // レイトレーシングの描画を行うパイプライン
        GLIB_PIPELINE_TYPE_MAX                                  // パイプラインの最大値
    }GLIB_PIPELINE_TYPE;

    // GLibを初期化する
    bool Init();

    // 描画開始
    void BeginRender(const GLIB_PIPELINE_TYPE& usePipelineType = GLIB_PIPELINE_TYPE_MAIN);

    // 描画終了
    void EndRender(const GLIB_PIPELINE_TYPE& usePipelineType = GLIB_PIPELINE_TYPE_MAIN);

    // GLibを解放する
    void Release();

    // 描画が完了するまで待機する
    void WaitDrawDone();

    // DeltaTimeを更新する
    void RefreshDeltaTime();

    // DeltaTimeを取得する
    float DeltaTime();

    // ウィンドウのスタイル
    enum GLIB_WINDOW_STYLE
    {
        GLIB_WINDOW_STYLE_DEFAULT = WS_OVERLAPPEDWINDOW,
        GLIB_WINDOW_STYLE_FULLSCREEN = WS_POPUP | WS_VISIBLE,
        GLIB_WINDOW_STYLE_BORDERLESS = WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        GLIB_WINDOW_STYLE_RESIZABLE = WS_OVERLAPPEDWINDOW | WS_THICKFRAME
    };

    // ウィンドウを表示する
    void ShowWindow();
    // ウィンドウの位置を設定する
    void SetWindowPos(int x, int y);
    // ウィンドウのアスペクト比を設定する
    void SetWindowAspect(float aspect);
    // ウィンドウのスタイルを設定する
    void SetWindowStyle(const GLIB_WINDOW_STYLE& style);
    // ウィンドウの名前を設定する
    void SetWindowName(const LPCWSTR& wndName);
    // ウィンドウのサイズを設定する
    void SetWindowSize(int width, int height);

    GLibWindow* GetWindow();
}