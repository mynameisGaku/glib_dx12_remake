#pragma once
#include <vector>
#include <list>
#include <GLibLogger.h>

namespace glib
{
	/// <summary>
	/// �|�C���^�[���폜����Bnull�`�F�b�N�ƁA�폜���nullptr�������Ă����B
	/// </summary>
	/// <param name="p">�폜�������|�C���^�[</param>
	template<class C>
	static inline void SafeDelete(C*& p)
	{
		if (p != nullptr)
		{
			delete p;
			p = nullptr;
			Logger::FormatDebugLog("SafeDelete: �|�C���^���폜���܂����B");
		}
		else
		{
			Logger::FormatWarningLog("SafeDelete: nullptr �̂��ߍ폜���X�L�b�v���܂����B");
		}
	}

	template <typename T>
	void SafeReleaseDX(T*& ptr)
	{
		if (ptr != nullptr)
		{
			ptr->Release();
			ptr = nullptr;
			Logger::FormatDebugLog("SafeReleaseDX: DirectX ���\�[�X��������܂����B");
		}
		else
		{
			Logger::FormatWarningLog("SafeReleaseDX: nullptr �̂��߉�����X�L�b�v���܂����B");
		}
	}
}
