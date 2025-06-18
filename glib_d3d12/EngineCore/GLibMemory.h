#pragma once
#include <vector>
#include <list>
#include <string>
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
			char buf[256];
			sprintf_s(buf, sizeof(buf), "%p", static_cast<void*>(p));
			delete p;
			p = nullptr;
			Logger::FormatDebugLog("SafeDelete: �|�C���^���폜���܂����B0x%s", buf);
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
			char buf[256];
			sprintf_s(buf, sizeof(buf), "%p", static_cast<void*>(ptr));
			ptr->Release();
			ptr = nullptr;
			Logger::FormatDebugLog("SafeReleaseDX: DirectX ���\�[�X��������܂����B0x%s", buf);
		}
		else
		{
			Logger::FormatWarningLog("SafeReleaseDX: nullptr �̂��߉�����X�L�b�v���܂����B");
		}
	}
}
