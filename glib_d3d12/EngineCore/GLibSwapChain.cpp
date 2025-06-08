#include <GLibSwapChain.h>
#include <GLibLogger.h>
#include <GLibWindow.h>
#include <GLibCommandQueue.h>
#include <GLibDescriptorPool.h>
#include <GLibGraphicsCommandList.h>
#include <GLibCommandAllocator.h>
#include <GLibFence.h>
#include <GLibDevice.h>
#include <GLib.h>

glib::GLibSwapChain::~GLibSwapChain()
{
    for (auto& buffer : m_BackBuffers)
    {
        if (buffer)
        {
            buffer.Reset();
        }
    }
    m_BackBuffers.clear();
    
    if (m_SwapChain)
    {
        m_SwapChain.Reset();
    }
    
    glib::Logger::DebugLog("Swap chain resources released successfully.");

    m_pDevice = nullptr;
    m_pCommandQueue = nullptr;
    m_pCommandAllocator = nullptr;
    m_pDescriptorPool = nullptr;

    glib::Logger::DebugLog("GLibSwapChain destroyed successfully.");
}

bool glib::GLibSwapChain::Initialize(GLibDevice* device, GLibCommandQueue* queue, GLibCommandAllocator* allocator, GLibDescriptorPool* pool, UINT buffIdx)
{

    m_pDevice                   = device;
    m_pCommandQueue             = queue;
    m_pCommandAllocator         = allocator;
    m_pDescriptorPool           = pool;






    // Swapchain���
    {
        IDXGIFactory4* factory = nullptr;
        m_Hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
        if (FAILED(m_Hr))
        {
            glib::Logger::FormatErrorLog("Failed to create DXGIFactory. HRESULT: 0x{%X}", m_Hr);
            return false;
        }
        glib::Logger::DebugLog("DXGIFactory created successfully.");

        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.BufferCount = buffIdx; // �o�b�N�o�b�t�@�̐�
        desc.Width = glib::GetWindow()->GetClientWidth(); // �E�B���h�E�̕�
        desc.Height = glib::GetWindow()->GetClientHeight(); // �E�B���h�E�̍���
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �o�b�N�o�b�t�@�̃t�H�[�}�b�g
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // �����_�[�^�[�Q�b�g�Ƃ��Ďg�p
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // �X���b�v����
        desc.SampleDesc.Count = 1; // �}���`�T���v���Ȃ�
        desc.SampleDesc.Quality = 0; // �}���`�T���v���̕i��

        IDXGISwapChain1* swapChain = nullptr;
        m_Hr = factory->CreateSwapChainForHwnd(
            m_pCommandQueue->Get(),
            glib::GetWindow()->GetHWnd(),
            &desc,
            nullptr,
            nullptr,
            &swapChain
        );

        if (FAILED(m_Hr))
        {
            glib::Logger::FormatErrorLog("Failed to create swap chain. HRESULT: 0x{%X}", m_Hr);
            factory->Release();
            return false;
        }

        // Query IDXGISwapChain4 Interface
        m_Hr = swapChain->QueryInterface(IID_PPV_ARGS(m_SwapChain.GetAddressOf()));
        if (FAILED(m_Hr))
        {
            glib::Logger::FormatErrorLog("Failed to query IDXGISwapChain4 interface. HRESULT: 0x{%X}", m_Hr);
            swapChain->Release();
            factory->Release();
            return false;
        }
        swapChain->Release(); // QueryInterface�Ŏ擾�����̂ŁA�����ł�Release����
        factory->Release(); // Factory��Release
    }

    glib::Logger::DebugLog("Swap chain created successfully.");

    // �o�b�N�o�b�t�@�r���[�̓��ꕨ�ł���DescriptorHeap���擾
    {
        if (!m_BbvHeap.Initialize(m_pDescriptorPool, m_pDescriptorPool->Get(glib::GLIB_DESCRIPTOR_HEAP_TYPE_RTV)->Get()))
        {
            glib::Logger::FormatErrorLog("Failed to create RTV descriptor heap. HRESULT: 0x{%X}", m_Hr);
            return false;
        }
        glib::Logger::DebugLog("RTV descriptor heap created successfully.");
    }

    // �o�b�N�o�b�t�@�r���[���f�B�X�N���v�^�q�[�v�ɍ��
    {
        D3D12_CPU_DESCRIPTOR_HANDLE hBbvHeap = m_BbvHeap.Get()->GetCPUDescriptorHandleForHeapStart();
        m_BbvHeapSize = m_pDevice->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        for (UINT i = 0; i < buffIdx; ++i)
        {
            m_BackBuffers.push_back(nullptr); // �o�b�N�o�b�t�@�̏�����
            m_Hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i]));
            if (FAILED(m_Hr))
            {
                glib::Logger::FormatErrorLog("Failed to get swap chain buffer {}. HRESULT: 0x{%X}", i, m_Hr);
                return false;
            }
            hBbvHeap.ptr += i * m_BbvHeapSize;
            m_pDevice->Get()->CreateRenderTargetView(m_BackBuffers[i].Get(), nullptr, hBbvHeap);
            glib::Logger::FormatDebugLog("Render target view for buffer {%d} created successfully.", i);
        }
    }

    glib::Logger::DebugLog("All render target views created successfully.");

    return true;
}

void glib::GLibSwapChain::DrawBegin(glib::GLibGraphicsCommandList* cmdList)
{
    // �o�b�N�o�b�t�@���N���A����
    {
        cmdList->Get()->Reset(m_pCommandAllocator->Get(), nullptr);

        m_BackBufIdx = m_SwapChain->GetCurrentBackBufferIndex();

        // �o���A�Ńo�b�N�o�b�t�@��`��^�[�Q�b�g�ɐ؂�ւ���
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_BackBuffers[m_BackBufIdx].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->Get()->ResourceBarrier(1, &barrier);

        // �o�b�N�o�b�t�@�̏ꏊ���w���f�B�X�N���v�^�q�[�v�n���h����p�ӂ���
        auto hBbvHeap = m_BbvHeap.Get()->GetCPUDescriptorHandleForHeapStart();
        hBbvHeap.ptr += m_BackBufIdx * m_BbvHeapSize;
        // �o�b�N�o�b�t�@��`��^�[�Q�b�g�Ƃ��Đݒ肷��
        cmdList->Get()->OMSetRenderTargets(1, &hBbvHeap, FALSE, nullptr);
        // �`��^�[�Q�b�g�̃N���A
        static float radian = .0f;
        float r = cos(radian) * 0.5f + 0.5f;
        float g = 0.25f;
        float b = 0.5f;
        const float clearColor[4] = { r, g, b, 1.0f };
        radian += 1.00f * glib::DeltaTime(); // �F��ς��邽�߂ɏ�������radian�𑝂₷
        cmdList->Get()->ClearRenderTargetView(hBbvHeap, clearColor, 0, nullptr);
    }
}

void glib::GLibSwapChain::DrawEnd(glib::GLibGraphicsCommandList* cmdList)
{
    // �o���A�Ńo�b�N�o�b�t�@��\���p�ɐ؂�ւ���
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_BackBuffers[m_BackBufIdx].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->Get()->ResourceBarrier(1, &barrier);

    // �R�}���h���X�g�N���[�Y
    m_Hr = cmdList->Get()->Close();
    if (FAILED(m_Hr))
    {
        glib::Logger::FormatErrorLog("Failed to close command list. HRESULT: 0x{%X}", m_Hr);
        return;
    }
    glib::Logger::DebugLog("Command list closed successfully.");

    // �R�}���h���X�g�����s
    ID3D12CommandList* commandLists[] = { cmdList->Get() };
    m_pCommandQueue->Get()->ExecuteCommandLists(_countof(commandLists), commandLists);

    // �`�抮����҂�
    glib::WaitDrawDone();

    // �o�b�N�o�b�t�@��\��
    // Present
    int vsync = m_BackBufIdx == 3 ? 0 : 1;
    m_Hr = m_SwapChain->Present(vsync, 0); // VSync��L���ɂ��ĕ\��
    if (FAILED(m_Hr))
    {
        glib::Logger::FormatErrorLog("Failed to present swap chain. HRESULT: 0x{%X}", m_Hr);
        return;
    }
    glib::Logger::DebugLog("Swap chain presented successfully.");

    // �R�}���h�A���P�[�^��҂�
    m_Hr = m_pCommandAllocator->Get()->Reset();
    if (FAILED(m_Hr))
    {
        glib::Logger::FormatErrorLog("Failed to reset command allocator. HRESULT: 0x{&X}", m_Hr);
        return;
    }
    glib::Logger::DebugLog("Command allocator reset successfully.");
}
