#pragma once

#include <GLibWindow.h>
#include <GLibLogger.h>

namespace glib
{
    // GLibを初期化する
    bool Init();

    // 描画開始
    void BeginRender();

    // 描画終了
    void EndRender();

    // GLibを解放する
    void Release();

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
}