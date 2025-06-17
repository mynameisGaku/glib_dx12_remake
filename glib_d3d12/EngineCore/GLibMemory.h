#pragma once
#include <vector>
#include <list>

namespace glib
{
	/// <summary>
	/// ポインターを削除する。nullチェックと、削除後にnullptrを代入してくれる。
	/// </summary>
	/// <param name="p">削除したいポインター</param>
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