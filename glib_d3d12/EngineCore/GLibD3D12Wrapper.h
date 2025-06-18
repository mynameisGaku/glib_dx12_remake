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

        struct alignas(256) Transform
        {
            XMMATRIX World;
            XMMATRIX View;
            XMMATRIX Proj;
        };

        template<typename T>
        struct ConstantBufferView
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;
            D3D12_CPU_DESCRIPTOR_HANDLE     HandleCPU;
            D3D12_GPU_DESCRIPTOR_HANDLE     HandleGPU;
            T*                              pBuffer;
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
        D3D12_CPU_DESCRIPTOR_HANDLE                     m_HandleDSV{};
        GLibComPtr<ID3D12RootSignature>                 m_Rootsignature{};
        GLibComPtr<ID3D12PipelineState>                 m_PSO{};


        GLibComPtr<ID3D12Resource>                      m_VB{};
        D3D12_VERTEX_BUFFER_VIEW                        m_VBV{};
        GLibComPtr<ID3D12Resource>                      m_IB{};
        D3D12_INDEX_BUFFER_VIEW                         m_IBV{};
        UINT32 ibvcount;
        vector<GLibComPtr<ID3D12Resource>>              m_CB{};
        vector<ConstantBufferView<Transform>>           m_CBVs{};
        GLibComPtr<ID3D12Resource>                      m_DepthBuffer{};


        XMFLOAT4                                        m_ClearColor;
        float m_Rotate;

        /* profiler */
        LARGE_INTEGER m_Freq{};
        LARGE_INTEGER m_Start{}, m_End{};
        bool m_RefreshTick = false;

        /* methods */
        bool init();
        void term();
        bool initD3D();
        void termD3D();
        void present(UINT32 interval);
    public:
        /* methods */
        void BeginRender();
        void EndRender();

        void BeginRecordPerformance();
        void EndRecordPerformance();
        void RunProfile();

        void WaitGpu();

        void SetClearColor(const XMFLOAT4& color);
    };
}