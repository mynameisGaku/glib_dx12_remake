#include "GLibD3D12Wrapper.h"
#include "GLibWindow.h"
#include "GLibMemory.h"
#include "GLibDebug.h"

#include <cassert>

glib::GLibD3D12Wrapper::GLibD3D12Wrapper()
{
    assert(init());
}

glib::GLibD3D12Wrapper::~GLibD3D12Wrapper()
{
    term();
}

bool glib::GLibD3D12Wrapper::init()
{
    assert(initD3D());

    {
        // ���_�f�[�^
        Vertex vertices[] =
        {
            { XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
            { XMFLOAT3( 1.0f, -1.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
            { XMFLOAT3( 0.0f,  1.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        };

        // �q�[�v�v���p�e�B
        D3D12_HEAP_PROPERTIES prop{};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // ���\�[�X�ݒ�
        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = sizeof(vertices);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // ���\�[�X����
        HRESULT hr{};
        hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_VB.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create resource. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created resource.");

        // �}�b�v
        void* ptr = nullptr;
        hr = m_VB->Map(0, nullptr, &ptr);
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to mapped resource. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully mapped resource.");

        // ���_�f�[�^���}�b�v���
        memcpy(ptr, vertices, sizeof(vertices));

        // �}�b�v����
        m_VB->Unmap(0, nullptr);

        // ���_�o�b�t�@�r���[�̐ݒ�
        m_VBV.BufferLocation = m_VB->GetGPUVirtualAddress();
        m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));
        m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));
    }

    // �f�B�X�N���v�^�q�[�v�̐���
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 5000;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;

        HRESULT hr{};
        hr = m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create descriptor heap. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully create descriptor heap.");
    }

    // �萔�o�b�t�@����
    {
        D3D12_HEAP_PROPERTIES prop{};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // �ݒ�
        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = sizeof(Transform);
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        auto incrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        for (int i = 0; i < m_FrameCount; i++)
        {
            // ���\�[�X����
            auto hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_CBVs[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("Failed to create resource. HRESULT=0x%x", hr);
                return false;
            }
            glib::Logger::FormatDebugLog("Successfully create resource.");
        }
    }

    return true;
}

void glib::GLibD3D12Wrapper::term()
{
    termD3D();
}

bool glib::GLibD3D12Wrapper::initD3D()
{
    /*
    �f�o�C�X
    ��
    �R�}���h�L���[
    ��
    �X���b�v�`�F�C��
    ��
    �R�}���h�A���P�[�^
    ��
    �R�}���h���X�g
    ��
    �����_�[�^�[�Q�b�g�r���[
    ��
    �t�F���X
    */

    HRESULT hr{};

    // Enable the debug layer
    glib::GLibDebug::EnableDebugLayer();

    // �f�o�C�X
    {
        hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create d3d12 device. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created d3d12 device.");
    }

    // �R�}���h�L���[
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Type       = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Priority   = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags      = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask   = 0;
        hr = m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create d3d12 command queue. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created d3d12 command queue.");
    }

    // �X���b�v�`�F�C��
    {
        // DXGI�t�@�N�g���[�̐���
        GLibComPtr<IDXGIFactory4> factory{};
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create dxgi factory4. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created dxgi factory4.");

        // �X���b�v�`�F�C���̐ݒ�
        DXGI_SWAP_CHAIN_DESC desc{};

        m_FrameCount = (UINT32)glib::GetBackBufferCount();

        desc.BufferDesc.Width                       = (UINT)glib::GetWindow()->GetClientWidth();
        desc.BufferDesc.Height                      = (UINT)glib::GetWindow()->GetClientHeight();
        desc.BufferDesc.RefreshRate.Numerator       = (UINT)glib::GetMaxFPS();
        desc.BufferDesc.RefreshRate.Denominator     = 1;
        desc.BufferDesc.ScanlineOrdering            = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        desc.BufferDesc.Scaling                     = DXGI_MODE_SCALING_UNSPECIFIED;
        desc.BufferDesc.Format                      = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count                       = 1;
        desc.SampleDesc.Quality                     = 0;
        desc.BufferUsage                            = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount                            = m_FrameCount;
        desc.OutputWindow                           = glib::GetWindow()->GetHWnd();
        desc.Windowed                               = TRUE;
        desc.SwapEffect                             = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags                                  = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // �X���b�v�`�F�C���̐���
        IDXGISwapChain* swapChain{};

        hr = factory->CreateSwapChain(m_CommandQueue.Get(), &desc, &swapChain);
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create dxgi swapchain. HRESULT=0x%x", hr);
            factory.Reset();
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created dxgi swapchain.");

        // IDXGISwapChain3���擾
        hr = swapChain->QueryInterface(IID_PPV_ARGS(m_SwapChain.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create dxgi swapchain3. HRESULT=0x%x", hr);
            factory.Reset();
            glib::SafeReleaseDX(swapChain);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created dxgi swapchain3.");

        // �o�b�N�o�b�t�@�ԍ����擾
        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        // �s�v�Ȃ̂ō폜
        factory.Reset();
        glib::SafeReleaseDX(swapChain);
    }

    // �R�}���h�A���P�[�^�[
    {
        for (UINT32 i = 0; i < m_FrameCount; i++)
        {
            m_CommandAllocators.push_back(GLibComPtr<ID3D12CommandAllocator>());
            hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocators[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("Failed to create d3d12 command allocator [%d]. HRESULT=0x%x", i, hr);
                return false;
            }
            glib::Logger::FormatDebugLog("Successfully created d3d12 command allocator.");
        }
    }

    // �R�}���h���X�g
    {
        hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[m_FrameIndex].Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create d3d12 command list. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created d3d12 command list.");
    }

    // �����_�[�^�[�Q�b�g�r���[
    {
        // �f�B�X�N���v�^�q�[�v�̐ݒ�
        D3D12_DESCRIPTOR_HEAP_DESC desc{};

        desc.NumDescriptors = m_FrameCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;

        for (int i = 0; i < GLIB_DESCRIPTOR_HEAP_TYPE_MAX; i++)
        {
            m_DescriptorHeaps.push_back(GLibComPtr<ID3D12DescriptorHeap>());
        }

        // �f�B�X�N���v�^�q�[�v����
        hr = m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_RTV].GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("Failed to create d3d12 rtv descriptor heap. HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("Successfully created d3d12 rtv descriptor heap.");

        auto handle = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_RTV]->GetCPUDescriptorHandleForHeapStart();
        auto incrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (auto i = 0u; i < m_FrameCount; i++)
        {
            m_ColorBuffers.push_back(GLibComPtr<ID3D12Resource>());
            hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_ColorBuffers[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("Failed to retrieve buffer from swap chain at index [%d]. HRESULT=0x%08X", i, hr);
                return false;
            }
            glib::Logger::FormatDebugLog("Successfully retrieved buffer from swap chain and created RTV descriptor.");

            D3D12_RENDER_TARGET_VIEW_DESC viewDesc{};
            viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipSlice = 0;
            viewDesc.Texture2D.PlaneSlice = 0;

            // �����_�[�^�[�Q�b�g�r���[����
            m_Device->CreateRenderTargetView(m_ColorBuffers[i].Get(), &viewDesc, handle);

            m_HandlesRTV.push_back(D3D12_CPU_DESCRIPTOR_HANDLE());
            m_HandlesRTV[i] = handle;
            handle.ptr += incrementSize;
        }
    }

    // �t�F���X
    {
        // �J�E���^�[���Z�b�g
        for (auto i = 0u; i < m_FrameCount; i++)
        {
            m_FenceCounters.push_back(UINT64());
            m_FenceCounters[i] = 0;
            glib::Logger::FormatDebugLog("Successfully fence counters[%d] reset.", i);
        }

        // �t�F���X����  
        hr = m_Device->CreateFence(m_FenceCounters[m_FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));  
        if (FAILED(hr))  
        {  
            glib::Logger::FormatCriticalLog("Failed to create d3d12 fence. HRESULT=0x%08X", hr);  
            return false;  
        }  
        glib::Logger::FormatDebugLog("Successfully created d3d12 fence.");

        m_FenceCounters[m_FrameIndex]++;

        // �C�x���g����  
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);  
        if (m_FenceEvent == nullptr)  
        {  
           glib::Logger::FormatCriticalLog("Failed to create fence event.");  
           return false;  
        }  
        glib::Logger::FormatDebugLog("Successfully created fence event.");
    }

    // �R�}���h���X�g����
    m_CommandList->Close();

    return true;
}

void glib::GLibD3D12Wrapper::waitGpu()
{
    assert(m_CommandQueue);
    assert(m_Fence);
    assert(m_FenceEvent);

    // �V�O�i��
    m_CommandQueue->Signal(m_Fence.Get(), m_FenceCounters[m_FrameIndex]);

    // �������ɃC�x���g�Z�b�g
    m_Fence->SetEventOnCompletion(m_FenceCounters[m_FrameIndex], m_FenceEvent);

    // �ҋ@����
    WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

    // �J�E���^�[�t���X
    m_FenceCounters[m_FrameIndex]++;
}

void glib::GLibD3D12Wrapper::present(UINT32 interval)
{
    HRESULT hr{};
    hr = m_SwapChain->Present(interval, 0);

    if (FAILED(hr))
    {
        glib::Logger::FormatWarningLog("Present failed. HRESULT=0x%08X", hr);
    }

    // �V�O�i��
    const auto currentValue = m_FenceCounters[m_FrameIndex];
    m_CommandQueue->Signal(m_Fence.Get(), currentValue);

    // �o�b�N�o�b�t�@�ԍ��X�V
    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

    // ���̃t���[���̕`�揀�����܂��Ȃ�ҋ@
    if (m_Fence->GetCompletedValue() < m_FenceCounters[m_FrameIndex])
    {
        m_Fence->SetEventOnCompletion(m_FenceCounters[m_FrameIndex], m_FenceEvent);
        WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
    }

    // ���̃t���[���̃t�F���X�J�E���^�[���₷
    m_FenceCounters[m_FrameIndex] = currentValue + 1;
}

void glib::GLibD3D12Wrapper::termD3D()
{
    waitGpu();

    CloseHandle(m_FenceEvent);
    m_FenceEvent = nullptr;

    m_Fence.Reset();
    m_CommandList.Reset();
    for (auto& allocator : m_CommandAllocators) allocator.Reset();
    for (auto& resource : m_ColorBuffers) resource.Reset();
    for (auto& heap : m_DescriptorHeaps) heap.Reset();
    m_SwapChain.Reset();
    m_CommandQueue.Reset();
    m_Device.Reset();
}

void glib::GLibD3D12Wrapper::BeginRender()
{
    // �R�}���h�L�^�J�n
    m_CommandAllocators[m_FrameIndex]->Reset();
    m_CommandList->Reset(m_CommandAllocators[m_FrameIndex].Get(), nullptr);

    // ���\�[�X�o���A�iPresent �� RenderTarget�j
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_ColorBuffers[m_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // ���\�[�X�o���A
    m_CommandList->ResourceBarrier(1, &barrier);

    // �����_�[�^�[�Q�b�g�ݒ�
    m_CommandList->OMSetRenderTargets(1, &m_HandlesRTV[m_FrameIndex], FALSE, nullptr);

    // �����_�[�^�[�Q�b�g�r���[�̃N���A
    m_CommandList->ClearRenderTargetView(m_HandlesRTV[m_FrameIndex], &m_ClearColor.x, 0, nullptr);
}

void glib::GLibD3D12Wrapper::EndRender()
{
    // ���\�[�X�o���A�iRenderTarget �� Present�j
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_ColorBuffers[m_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_CommandList->ResourceBarrier(1, &barrier);

    // �R�}���h���X�g�̋L�^�I��
    m_CommandList->Close();

    // ���s
    ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(1, ppCommandLists);

    // Present
    present(1);

    // �t�F���X����
    m_CommandQueue->Signal(m_Fence.Get(), m_FenceCounters[m_FrameIndex]);
    m_FenceCounters[m_FrameIndex]++;

    // ���t���[����
    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

    waitGpu();
}

void glib::GLibD3D12Wrapper::SetClearColor(const XMFLOAT4& color)
{
    m_ClearColor = color;
}
