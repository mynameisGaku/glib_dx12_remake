#pragma once
#include <vector>
#include <list>

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
		}
	}

	template < typename T>
	void SafeReleaseDX(T*& ptr)
	{
		if (ptr != nullptr)
		{
			ptr->Release();
			ptr = nullptr;
		}
	}
}