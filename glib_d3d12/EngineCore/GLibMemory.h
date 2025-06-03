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
	static inline void SafeDelete(C*& p) {
		if (p != nullptr) {
			delete p;
			p = nullptr;
		}
	}

	/// <summary>
	/// 配列のポインターを削除する。nullチェックと、削除後にnullptrを代入してくれる。
	/// </summary>
	/// <param name="p">削除したいポインター</param>
	template<class C>
	static inline void SafeDeleteArray(C*& p, int size) {
        for (int i = 0; i < size; ++i) {
            if (p[i] != nullptr) {
                delete p[i];
                p[i] = nullptr;
            }
        }
	}

	/// <summary>
	/// 配列のポインターを削除する。nullチェックと、削除後にnullptrを代入してくれる。
	/// </summary>
	/// <param name="p">削除したいポインター</param>
	template<class C>
	static inline void SafeDeleteList(std::list<C*>& p) {
		if (p.empty())
			return;

		for (auto itr = p.begin(); itr != p.end();) {
			delete* itr;
			itr = p.erase(itr);
		}

		p.clear();
	}

	/// <summary>
	/// 配列のポインターを削除する。nullチェックと、削除後にnullptrを代入してくれる。
	/// </summary>
	/// <param name="p">削除したいポインター</param>
	template<class C>
	static inline void SafeDeleteVector(std::vector<C*>& p) {
		if (p.empty())
			return;

		for (auto itr = p.begin(); itr != p.end();) {
			delete* itr;
			itr = p.erase(itr);
		}

		p.clear();
	}
}