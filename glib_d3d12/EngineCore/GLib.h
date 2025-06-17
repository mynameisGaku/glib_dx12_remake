#pragma once
// os
#include <windows.h>
#include <cstdint>

// d3d
#include <DirectXMath.h>
#include <d3dx12.h>
#include <d3d12.h>
#include <dxgi1_4.h>

/* pragma link */
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// profiler
#include <psapi.h>

#include <GLibWindow.h>
#include <GLibLogger.h>

#include <string>

/* vendor */
#include <Vendor/magic_enum/magic_enum.hpp>

namespace glib
{
    // ディスクリプタヒープの種類
    typedef enum
    {
        GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV = 0,                // CBV/SRV/UAV用のディスクリプタヒープ
        GLIB_DESCRIPTOR_HEAP_TYPE_SAMPLER,                        // サンプラーディスクリプタヒープ
        GLIB_DESCRIPTOR_HEAP_TYPE_RTV,                            // レンダーターゲットビュー用のディスクリプタヒープ
        GLIB_DESCRIPTOR_HEAP_TYPE_DSV,                            // 深度ステンシルビュー用のディスクリプタヒープ
        GLIB_DESCRIPTOR_HEAP_TYPE_MAX                             // ディスクリプタヒープの最大値
    }GLIB_DESCRIPTOR_HEAP_TYPE;

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
    // ウィンドウのポインタを取得する
    GLibWindow* GetWindow();

    D3D12_VIEWPORT GetViewport();
    D3D12_RECT GetRect();

    void SetBackBufferCount(int count);

    int GetBackBufferCount();

    // EnumをStringに変換する
    template <typename T>
    std::string EnumToString(T _enum)
    {
        return std::string(magic_enum::enum_name(_enum));
    }

    std::wstring StringToWString(const std::string& str);
    std::string WStringToString(const std::wstring& wstr);

    void RunProfile();

    void BeginRecordPerformance();
    void EndRecordPerformance();

    void SetMaxFPS(int fps);
    int GetMaxFPS();
}