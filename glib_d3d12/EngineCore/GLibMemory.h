#pragma once
#include <vector>
#include <list>
#include <string>
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
			char buf[256];
			sprintf_s(buf, sizeof(buf), "%p", static_cast<void*>(p));
			delete p;
			p = nullptr;
			Logger::FormatDebugLog("SafeDelete: ポインタを削除しました。0x%s", buf);
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
			char buf[256];
			sprintf_s(buf, sizeof(buf), "%p", static_cast<void*>(ptr));
			ptr->Release();
			ptr = nullptr;
			Logger::FormatDebugLog("SafeReleaseDX: DirectX リソースを解放しました。0x%s", buf);
		}
		else
		{
			Logger::FormatWarningLog("SafeReleaseDX: nullptr のため解放をスキップしました。");
		}
	}
}
