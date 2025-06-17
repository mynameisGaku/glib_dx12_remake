#pragma once
#include <vector>
#include <list>
#include <GLibLogger.h>

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
			Logger::FormatDebugLog("SafeDelete: ポインタを削除しました。");
		}
		else
		{
			Logger::FormatWarningLog("SafeDelete: nullptr のため削除をスキップしました。");
		}
	}

	template <typename T>
	void SafeReleaseDX(T*& ptr)
	{
		if (ptr != nullptr)
		{
			ptr->Release();
			ptr = nullptr;
			Logger::FormatDebugLog("SafeReleaseDX: DirectX リソースを解放しました。");
		}
		else
		{
			Logger::FormatWarningLog("SafeReleaseDX: nullptr のため解放をスキップしました。");
		}
	}
}
