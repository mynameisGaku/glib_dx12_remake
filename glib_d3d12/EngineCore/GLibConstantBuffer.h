#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>
#include <GLibDevice.h>
#include <GLibDescriptorPool.h>
#include <Windows.h>
#include <type_traits>

namespace glib
{
    class GLibConstantBuffer
    {
    public:

        GLibConstantBuffer() : m_Index(0) {}
        ~GLibConstantBuffer();
        bool Initialize(GLibDevice* device, GLibDescriptorPool* pPool, const D3D12_RESOURCE_DESC& desc);

        template<typename T>
        void Update(const T& data)
        {
            if (m_pMappedConstBuf)
            {
                memcpy(m_pMappedConstBuf, &data, sizeof(T));
            }
            else
            {
                glib::Logger::ErrorLog("Constant buffer is not mapped.");
            }
        }

        ID3D12Resource* GetResource() const
        {
            return m_ConstBuf.Get();
        }

        ID3D12DescriptorHeap* GetDescriptorHeap() const
        {
            return m_pCbvHeap;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const
        {
            return m_hCbvHeap;
        }

        template<typename T>
        T* GetMappedBuffer()
        {
            static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
            return reinterpret_cast<T*>(m_pMappedConstBuf);
        }

    private:

        static UINT m_sCurrentIndex;
        UINT m_Index;

        void* m_pMappedConstBuf = nullptr;
        ComPtr<ID3D12Resource> m_ConstBuf = nullptr;
        ID3D12DescriptorHeap* m_pCbvHeap = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE m_hCbvHeap{};
        HRESULT m_Hr{};
    };
}