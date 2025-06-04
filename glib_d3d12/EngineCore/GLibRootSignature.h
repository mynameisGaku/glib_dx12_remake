#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibRootSignature
    {
    public:

        GLibRootSignature():m_Hr(0){}
        ~GLibRootSignature();

        // ルートシグネチャの初期化
        bool Initialize(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& rootSignatureDesc);

        // ルートシグネチャを取得
        ID3D12RootSignature* Get() const { return m_Rootsignature.Get(); }

    private:
        ComPtr<ID3D12RootSignature> m_Rootsignature;
        HRESULT m_Hr;
    };
}