#pragma once

#include <GLibWindow.h>
#include <GLibLogger.h>

namespace glib
{
    // GLib������������
    bool Init();

    // �`��J�n
    void BeginRender();

    // �`��I��
    void EndRender();

    // GLib���������
    void Release();

    // �E�B���h�E�̃X�^�C��
    enum GLIB_WINDOW_STYLE
    {
        GLIB_WINDOW_STYLE_DEFAULT = WS_OVERLAPPEDWINDOW,
        GLIB_WINDOW_STYLE_FULLSCREEN = WS_POPUP | WS_VISIBLE,
        GLIB_WINDOW_STYLE_BORDERLESS = WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        GLIB_WINDOW_STYLE_RESIZABLE = WS_OVERLAPPEDWINDOW | WS_THICKFRAME
    };

    // �E�B���h�E��\������
    void ShowWindow();
    // �E�B���h�E�̈ʒu��ݒ肷��
    void SetWindowPos(int x, int y);
    // �E�B���h�E�̃A�X�y�N�g���ݒ肷��
    void SetWindowAspect(float aspect);
    // �E�B���h�E�̃X�^�C����ݒ肷��
    void SetWindowStyle(const GLIB_WINDOW_STYLE& style);
    // �E�B���h�E�̖��O��ݒ肷��
    void SetWindowName(const LPCWSTR& wndName);
    // �E�B���h�E�̃T�C�Y��ݒ肷��
    void SetWindowSize(int width, int height);
}