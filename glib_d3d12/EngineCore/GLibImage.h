#pragma once
#include <GLibConstantBuffer.h>
#include <GLibGraphicsCommandList.h>
#include <GLibDescriptorPool.h>
#include <GLibDescriptorHeap.h>
#include <GLibTexture.h>
#include <GLibDevice.h>

#include <string>
#include <DirectXMath.h>
using namespace DirectX;


namespace glib
{
    class GLibImage
    {
    public:
        GLibImage();
        ~GLibImage();

        bool Initialize(GLibDevice* pDevice, GLibDescriptorPool* pPool, GLibGraphicsCommandList* pCmdList, const std::string& filepath);

        void Draw();


        GLibConstantBuffer* GetPositionBuffer() const { return m_pPositionBuf; }
        GLibConstantBuffer* GetDiffuseBuffer() const { return m_pDiffuseBuf; }
        GLibTexture* GetTextureBuffer() const { return m_pTextureBuf; }

    private:
        GLibDevice*                     m_pDevice           = nullptr;
        GLibGraphicsCommandList*        m_pCmdList          = nullptr;
        GLibDescriptorPool*             m_pDescriptorPool   = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE     m_hPositionHeap     {};
        UINT                            m_PositionHeapIndex {};
        D3D12_CPU_DESCRIPTOR_HANDLE     m_hDiffuseHeap      {};
        UINT                            m_DiffuseHeapIndex  {};
        D3D12_CPU_DESCRIPTOR_HANDLE     m_hTextureHeap      {};
        UINT                            m_TextureHeapIndex  {};
        GLibConstantBuffer*             m_pPositionBuf      = nullptr;
        GLibConstantBuffer*             m_pDiffuseBuf       = nullptr;
        GLibTexture*                    m_pTextureBuf       = nullptr;
    };
}