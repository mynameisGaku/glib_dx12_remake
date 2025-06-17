#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dx12.h>
#include <DirectXMath.h>

/* Utils */
#include <d3dx12.h>
#include <GLib.h>
#include <GLibComPtr.h>
#include <vector>

using namespace std;
using namespace DirectX;

namespace glib
{
    class GLibD3D12Wrapper
    {
    public:
        GLibD3D12Wrapper();
        ~GLibD3D12Wrapper();
    private:

        struct Vertex
        {
            XMFLOAT3 Position;
            XMFLOAT4 Color;
        };

        struct Transform
        {
            XMMATRIX World;
            XMMATRIX View;
            XMMATRIX Proj;
        };

        /* members */
        UINT32                                          m_FrameCount{};
        GLibComPtr<ID3D12Device>                        m_Device{};
        GLibComPtr<ID3D12CommandQueue>                  m_CommandQueue{};
        vector<GLibComPtr<ID3D12CommandAllocator>>      m_CommandAllocators{};
        GLibComPtr<ID3D12GraphicsCommandList>           m_CommandList{};
        GLibComPtr<IDXGISwapChain3>                     m_SwapChain{};
        vector<GLibComPtr<ID3D12Resource>>              m_ColorBuffers{};
        vector<GLibComPtr<ID3D12DescriptorHeap>>        m_DescriptorHeaps{};
        GLibComPtr<ID3D12Fence>                         m_Fence{};
        HANDLE                                          m_FenceEvent{};
        vector<UINT64>                                  m_FenceCounters{};
        UINT32                                          m_FrameIndex{};
        vector<D3D12_CPU_DESCRIPTOR_HANDLE>             m_HandlesRTV{};
        GLibComPtr<ID3D12RootSignature>                 m_Rootsignature{};
        GLibComPtr<ID3D12PipelineState>                 m_PSO{};


        GLibComPtr<ID3D12Resource>                      m_VB{};
        D3D12_VERTEX_BUFFER_VIEW                        m_VBV{};
        //vector<ConstantBufferView<Transform>>         m_CBVs{};


        XMFLOAT4                                        m_ClearColor;

        /* methods */
        bool init();
        void term();
        bool initD3D();
        void termD3D();
        void waitGpu();
        void present(UINT32 interval);
    public:
        /* methods */
        void BeginRender();
        void EndRender();

        void SetClearColor(const XMFLOAT4& color);
    };
}