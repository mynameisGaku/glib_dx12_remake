#pragma once
#include <d3d12.h>
#include <Windows.h>
#include <type_traits>

#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibDevice;
    class GLibDescriptorPool;

    class GLibConstantBuffer
    {
    public:

        GLibConstantBuffer() {}
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

        ID3D12Resource* GetResource() const;

        ID3D12DescriptorHeap* GetDescriptorHeap() const;

        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const;

        template<typename T>
        T* GetMappedBuffer()
        {
            static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
            return reinterpret_cast<T*>(m_pMappedConstBuf);
        }

    private:

        void* m_pMappedConstBuf = nullptr;
        ComPtr<ID3D12Resource> m_ConstBuf = nullptr;
        ID3D12DescriptorHeap* m_pCbvHeap = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE m_hCbvHeap{};
        HRESULT m_Hr{};
    };
}