#pragma once

#include <GLibWindow.h>
#include <GLibLogger.h>

namespace glib
{
    // �`��p�C�v���C���̎��
    typedef enum
    {
        GLIB_PIPELINE_TYPE_MAIN = 0,                            // �ʏ�̕`����s���p�C�v���C��
        GLIB_PIPELINE_TYPE_SHADOW,                              // 3D�v���~�e�B�u�̉e�`����s���p�C�v���C��
        GLIB_PIPELINE_TYPE_MAIN_NO_SHADOW,                      // �ʏ�̕`����s�����A�e�̉e�����󂯂Ȃ��p�C�v���C��
        GLIB_PIPELINE_TYPE_POSTPROCESS,                         // �|�X�g�v���Z�X(��ʌ���)�̕`����s���p�C�v���C��
        GLIB_PIPELINE_TYPE_MAIN_NO_POSTPROCESS_AND_SHADOW,      // �ʏ�̕`����s�����A�|�X�g�v���Z�X�Ɖe�̉e�����󂯂Ȃ��p�C�v���C��
        GLIB_PIPELINE_TYPE_UI,                                  // UI�̕`����s���p�C�v���C��
        GLIB_PIPELINE_TYPE_COMPUTE,                             // �R���s���[�g�V�F�[�_�[���g�p�����v�Z���s���p�C�v���C��
        GLIB_PIPELINE_TYPE_RAYTRACING,                          // ���C�g���[�V���O�̕`����s���p�C�v���C��
        GLIB_PIPELINE_TYPE_MAX                                  // �p�C�v���C���̍ő�l
    }GLIB_PIPELINE_TYPE;

    // GLib������������
    bool Init();

    // �`��J�n
    void BeginRender(const GLIB_PIPELINE_TYPE& usePipelineType = GLIB_PIPELINE_TYPE_MAIN);

    // �`��I��
    void EndRender(const GLIB_PIPELINE_TYPE& usePipelineType = GLIB_PIPELINE_TYPE_MAIN);

    // GLib���������
    void Release();

    // �`�悪��������܂őҋ@����
    void WaitDrawDone();

    // DeltaTime���X�V����
    void RefreshDeltaTime();

    // DeltaTime���擾����
    float DeltaTime();

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

    GLibWindow* GetWindow();
}