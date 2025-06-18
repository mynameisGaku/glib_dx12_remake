#include "GLibD3D12Wrapper.h"
#include "GLibWindow.h"
#include "GLibMemory.h"
#include "GLibDebug.h"
#include "GLibBinaryLoader.h"

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

    HRESULT hr{};

    // �f�B�X�N���v�^�q�[�v�̐���
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 5000;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;

        hr = m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("�f�B�X�N���v�^�q�[�v�̍쐬�Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("�f�B�X�N���v�^�q�[�v�̍쐬�ɐ������܂����B");
    }

    // �e�N�X�`������
    {
        ResourceUploadBatch batch(m_Device.Get());
        batch.Begin();

        // ����
        std::wstring path = L"Resources/Models/teapot/default.DDS";
        hr = CreateDDSTextureFromFile(m_Device.Get(), batch, path.c_str(), m_Texture.Resource.GetAddressOf(), true);
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("DDS�t�@�C���̓ǂݍ��݂Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("DDS�t�@�C���̓ǂݍ��݂ɐ������܂����B");

        auto future = batch.End(m_CommandQueue.Get());

        future.wait();

        auto incrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        auto handleCPU = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetCPUDescriptorHandleForHeapStart();
        auto handleGPU = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetGPUDescriptorHandleForHeapStart();

        handleCPU.ptr += incrementSize * 2;
        handleGPU.ptr += incrementSize * 2;

        m_Texture.HandleCPU = handleCPU;
        m_Texture.HandleGPU = handleGPU;

        auto texDesc = m_Texture.Resource->GetDesc();

        // SRV�̐ݒ�
        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
        viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        viewDesc.Format = texDesc.Format;
        viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        viewDesc.Texture2D.MostDetailedMip = 0;
        viewDesc.Texture2D.MipLevels = texDesc.MipLevels;
        viewDesc.Texture2D.PlaneSlice = 0;
        viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        // SRV����
        m_Device->CreateShaderResourceView(m_Texture.Resource.Get(), &viewDesc, handleCPU);

    }

    // ���b�V�����[�h
    {
        std::wstring path = L"Resources/Models/teapot/teapot.obj";
        if (not LoadMesh(path.c_str(), m_Meshes, m_Materials))
        {
            glib::Logger::FormatErrorLog("���b�V���̓ǂݍ��݂Ɏ��s���܂����Bfile: %s", path.c_str());
            return false;
        }
        glib::Logger::FormatDebugLog("���b�V���̓ǂݍ��݂ɐ������܂����Bfile: %s", path.c_str());
    }

    // ���_�o�b�t�@
    {
        auto& vertices = m_Meshes[0].Vertices;
        auto vbSize = sizeof(GLibMeshVertex) * vertices.size();

        // �q�[�v�ݒ�
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;

        // ���\�[�X�ݒ�
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width = vbSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        // ���\�[�X�쐬
        hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_VB.GetAddressOf()));
        assert(SUCCEEDED(hr));

        // �f�[�^�]��
        void* ptr = nullptr;
        m_VB->Map(0, nullptr, &ptr);
        memcpy(ptr, vertices.data(), vbSize);
        m_VB->Unmap(0, nullptr);

        // VBV�ݒ�
        m_VBV.BufferLocation = m_VB->GetGPUVirtualAddress();
        m_VBV.SizeInBytes = static_cast<UINT>(vbSize);
        m_VBV.StrideInBytes = sizeof(GLibMeshVertex);
    }


    // �C���f�b�N�X�o�b�t�@
    {

        // �C���f�b�N�X�f�[�^  
        auto size = sizeof(UINT32) * m_Meshes[0].Indices.size();
        auto indices = m_Meshes[0].Indices.data();

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
        desc.Width = size;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // ���\�[�X����
        hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_IB.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("���\�[�X�̐����Ɏ��s���܂����BHRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("���\�[�X�̐����ɐ������܂����B");

        // �}�b�v
        void* ptr = nullptr;
        hr = m_IB->Map(0, nullptr, &ptr);
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("���\�[�X�̃}�b�v�Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("���\�[�X�̃}�b�v�ɐ������܂����B");

        // ���_�f�[�^���}�b�v���
        memcpy(ptr, indices, size);

        // �}�b�v����
        m_IB->Unmap(0, nullptr);

        // ���_�o�b�t�@�r���[�̐ݒ�
        m_IBV.BufferLocation = m_IB->GetGPUVirtualAddress();
        m_IBV.Format = DXGI_FORMAT_R32_UINT;
        m_IBV.SizeInBytes = static_cast<UINT>(size);
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

        for (UINT32 i = 0; i < m_FrameCount * 2; i++)
        {
            m_CB.push_back(GLibComPtr<ID3D12Resource>());
            m_CBVs.push_back(ConstantBufferView<Transform>());
        }

        auto incrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        for (UINT32 i = 0; i < m_FrameCount * 2; i++)
        {
            // ���\�[�X����
            hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_CB[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("���\�[�X�̍쐬�Ɏ��s���܂����B HRESULT=0x%x", hr);
                return false;
            }
            glib::Logger::FormatDebugLog("���\�[�X�̍쐬�ɐ������܂����B");

            auto address = m_CB[i]->GetGPUVirtualAddress();
            auto handleCPU = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetCPUDescriptorHandleForHeapStart();
            auto handleGPU = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetGPUDescriptorHandleForHeapStart();

            handleCPU.ptr += incrementSize * i;
            handleGPU.ptr += incrementSize * i;

            // �萔�o�b�t�@�r���[�ݒ�
            m_CBVs[i].HandleCPU = handleCPU;
            m_CBVs[i].HandleGPU = handleGPU;
            m_CBVs[i].Desc.BufferLocation = address;
            m_CBVs[i].Desc.SizeInBytes = sizeof(Transform);

            // �萔�o�b�t�@�r���[����
            m_Device->CreateConstantBufferView(&m_CBVs[i].Desc, handleCPU);

            // �}�b�s���O
            hr = m_CB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBVs[i].pBuffer));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("���\�[�X�̃}�b�s���O�Ɏ��s���܂����B HRESULT=0x%x", hr);
                return false;
            }
            glib::Logger::FormatDebugLog("���\�[�X�̃}�b�s���O�ɐ������܂����B");

            auto eyePos = XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
            auto targetPos = XMVectorZero();
            auto upward = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

            auto fovY = XMConvertToRadians(37.5f);
            auto aspect = static_cast<float>(glib::GetWindow()->GetClientWidth()) / static_cast<float>(glib::GetWindow()->GetClientHeight());

            // �ϊ��s��
            m_CBVs[i].pBuffer->World = XMMatrixIdentity();
            m_CBVs[i].pBuffer->View = XMMatrixLookAtRH(eyePos, targetPos, upward);
            m_CBVs[i].pBuffer->Proj = XMMatrixPerspectiveFovRH(fovY, aspect, 0.01f, 100000.0f);
        }
    }

    // �[�x�o�b�t�@
    {
        // ���\�[�X
        D3D12_HEAP_PROPERTIES prop{};
        prop.Type = D3D12_HEAP_TYPE_DEFAULT;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC resDesc{};
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resDesc.Alignment = 0;
        resDesc.Width = glib::GetWindow()->GetClientWidth();
        resDesc.Height = glib::GetWindow()->GetClientHeight();
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels = 1;
        resDesc.Format = DXGI_FORMAT_D32_FLOAT;
        resDesc.SampleDesc.Count = 1;
        resDesc.SampleDesc.Quality = 0;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        
        D3D12_CLEAR_VALUE clearValue{};
        clearValue.Format = DXGI_FORMAT_D32_FLOAT;
        clearValue.DepthStencil.Depth = 1.0;
        clearValue.DepthStencil.Stencil = 0;

        hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &resDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(m_DepthBuffer.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("�[�x�o�b�t�@���\�[�X�̐����Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("�[�x�o�b�t�@���\�[�X�̐����ɐ������܂����B");

        // �f�B�X�N���v�^�q�[�v
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.NumDescriptors = 1;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = 0;

        hr = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_DSV].GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("�[�x�o�b�t�@�f�B�X�N���v�^�q�[�v�̐����Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("�[�x�o�b�t�@�f�B�X�N���v�^�q�[�v�����ɐ������܂����B");

        auto handle = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_DSV]->GetCPUDescriptorHandleForHeapStart();
        auto incrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        
        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc{};
        viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;
        viewDesc.Flags = D3D12_DSV_FLAG_NONE;

        // ����
        m_Device->CreateDepthStencilView(m_DepthBuffer.Get(), &viewDesc, handle);

        m_HandleDSV = handle;
    }

    // ���[�g�V�O�l�`������
    {
        auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        // ���[�g�p�����[�^�[
        D3D12_ROOT_PARAMETER param[2]{};
        param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        param[0].Descriptor.ShaderRegister = 0;
        param[0].Descriptor.RegisterSpace = 0;
        param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        // �����W
        D3D12_DESCRIPTOR_RANGE range{};
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors = 5000;
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = 0;
        
        // �p�����[�^�[2
        param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges = 1;
        param[1].DescriptorTable.pDescriptorRanges = &range;
        param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // �X�^�e�B�b�N�T���v��
        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
        sampler.MaxAnisotropy = 1;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = -D3D12_FLOAT32_MAX;
        sampler.MaxLOD = +D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // ���[�g�V�O�l�`��
        D3D12_ROOT_SIGNATURE_DESC desc{};
        desc.NumParameters = 2;
        desc.NumStaticSamplers = 1;
        desc.pParameters = param;
        desc.pStaticSamplers = &sampler;
        desc.Flags = flag;

        GLibComPtr<ID3DBlob> Blob = nullptr;
        GLibComPtr<ID3DBlob> ErrorBlob = nullptr;

        // �V���A���C�Y
        hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, Blob.GetAddressOf(), ErrorBlob.GetAddressOf());
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("���[�g�V�O�l�`���̃V���A���C�Y�Ɏ��s���܂����BHRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("���[�g�V�O�l�`���̃V���A���C�Y�ɐ������܂����B");

        // ���[�g�V�O�l�`������
        hr = m_Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(m_Rootsignature.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("���[�g�V�O�l�`���̐����Ɏ��s���܂����BHRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("���[�g�V�O�l�`���̐����ɐ������܂����B");
    }

    // �p�C�v���C���X�e�[�g����
    {
        // ���X�^���C�U�[�X�e�[�g�̐ݒ�
        D3D12_RASTERIZER_DESC descRS{};
        descRS.FillMode = D3D12_FILL_MODE_SOLID;
        descRS.CullMode = D3D12_CULL_MODE_NONE;
        descRS.FrontCounterClockwise = FALSE;
        descRS.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        descRS.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        descRS.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        descRS.DepthClipEnable = FALSE;
        descRS.MultisampleEnable = FALSE;
        descRS.AntialiasedLineEnable = FALSE;
        descRS.ForcedSampleCount = 0;
        descRS.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        
        // �����_�[�^�[�Q�b�g�̃u�����h�ݒ�
        D3D12_RENDER_TARGET_BLEND_DESC descRTBS =
        {
            FALSE, FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL
        };

        // �u�����h�X�e�[�g�̐ݒ�
        D3D12_BLEND_DESC descBS{};
        descBS.AlphaToCoverageEnable = FALSE;
        descBS.IndependentBlendEnable = FALSE;
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
        {
            descBS.RenderTarget[i] = descRTBS;
        }

        // �����ł���Ă��镡���̐ݒ�(DESC)�͑S�āA��̃p�C�v���C���̐ݒ�����Ă��܂��B
        // �ŏI�I�ɂ܂Ƃ߂Ă悤�₭�A��̃p�C�v���C�����o���オ��܂��B
        // �ׂ��Ȑݒ�́A�p�C�v���C���̎�ނɂ���ĈقȂ�̂ŁA���낢�뒲�ׂA���������߂Ă���p�C�v���C���ɃJ�X�^�����悤�B
#ifdef _DEBUG
        glib::GLibBinaryLoader vs("x64/Debug/SpriteVS.cso");
        glib::GLibBinaryLoader ps("x64/Debug/SpritePS.cso");
#else
        glib::GLibBinaryLoader vs("x64/Release/SpriteVS.cso");
        glib::GLibBinaryLoader ps("x64/Release/SpritePS.cso");
#endif

        // �[�x�X�e���V���X�e�[�g�̐ݒ�
        D3D12_DEPTH_STENCIL_DESC descDSS{};
        descDSS.DepthEnable = TRUE;
        descDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        descDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        descDSS.StencilEnable = FALSE;

        // �p�C�v���C���X�e�[�g�̐ݒ�
        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
        desc.InputLayout = GLibMeshVertex::InputLayout;
        desc.pRootSignature = m_Rootsignature.Get();
        desc.VS = {vs.Code(), vs.Size()};
        desc.PS = {ps.Code(), ps.Size()};
        desc.RasterizerState = descRS;
        desc.BlendState = descBS;
        desc.DepthStencilState = descDSS;
        desc.SampleMask = UINT_MAX;
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        // �p�C�v���C���X�e�[�g����
        hr = m_Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PSO.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("�p�C�v���C���X�e�[�g�̍쐬�Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("�p�C�v���C���X�e�[�g�̍쐬�ɐ������܂����B");
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
            glib::Logger::FormatCriticalLog("D3D12�f�o�C�X�̍쐬�Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("D3D12�f�o�C�X�̍쐬�ɐ������܂����B");
    }

    // �R�}���h�L���[
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;
        hr = m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("D3D12�R�}���h�L���[�̍쐬�Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("D3D12�R�}���h�L���[�̍쐬�ɐ������܂����B");
    }

    // �X���b�v�`�F�C��
    {
        // DXGI�t�@�N�g���[�̐���
        GLibComPtr<IDXGIFactory4> factory{};
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("DXGI�t�@�N�g���̍쐬�Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("DXGI�t�@�N�g���̍쐬�ɐ������܂����B");

        // �X���b�v�`�F�C���̐ݒ�
        DXGI_SWAP_CHAIN_DESC desc{};

        m_FrameCount = (UINT32)glib::GetBackBufferCount();

        desc.BufferDesc.Width = (UINT)glib::GetWindow()->GetClientWidth();
        desc.BufferDesc.Height = (UINT)glib::GetWindow()->GetClientHeight();
        desc.BufferDesc.RefreshRate.Numerator = (UINT)glib::GetMaxFPS();
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = m_FrameCount;
        desc.OutputWindow = glib::GetWindow()->GetHWnd();
        desc.Windowed = TRUE;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        // �X���b�v�`�F�C���̐���
        IDXGISwapChain* swapChain{};

        hr = factory->CreateSwapChain(m_CommandQueue.Get(), &desc, &swapChain);
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("DXGI�X���b�v�`�F�C���̍쐬�Ɏ��s���܂����B HRESULT=0x%x", hr);
            factory.Reset();
            return false;
        }
        glib::Logger::FormatDebugLog("DXGI�X���b�v�`�F�C���̍쐬�ɐ������܂����B");

        // IDXGISwapChain3���擾
        hr = swapChain->QueryInterface(IID_PPV_ARGS(m_SwapChain.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("DXGI�X���b�v�`�F�C��3�̍쐬�Ɏ��s���܂����B HRESULT=0x%x", hr);
            factory.Reset();
            glib::SafeReleaseDX(swapChain);
            return false;
        }
        glib::Logger::FormatDebugLog("DXGI�X���b�v�`�F�C��3�̍쐬�ɐ������܂����B");

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
                glib::Logger::FormatCriticalLog("D3D12�R�}���h�A���P�[�^�̍쐬�Ɏ��s���܂����B [%d]. HRESULT=0x%x", i, hr);
                return false;
            }
            glib::Logger::FormatDebugLog("D3D12�R�}���h�A���P�[�^�̍쐬�ɐ������܂����B");
        }
    }

    // �R�}���h���X�g
    {
        hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[m_FrameIndex].Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("D3D12�R�}���h���X�g�̍쐬�Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("D3D12�R�}���h���X�g�̍쐬�ɐ������܂����B");
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
            glib::Logger::FormatCriticalLog("D3D12 RTV�f�B�X�N���v�^�q�[�v�̍쐬�Ɏ��s���܂����B HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("D3D12 RTV�f�B�X�N���v�^�q�[�v�̍쐬�ɐ������܂����B");

        auto handle = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_RTV]->GetCPUDescriptorHandleForHeapStart();
        auto incrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (auto i = 0u; i < m_FrameCount; i++)
        {
            m_ColorBuffers.push_back(GLibComPtr<ID3D12Resource>());
            hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_ColorBuffers[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("�X���b�v�`�F�C������̃o�b�t�@�擾�Ɏ��s���܂����i�C���f�b�N�X [%d]. HRESULT=0x%08X", i, hr);
                return false;
            }
            glib::Logger::FormatDebugLog("�X���b�v�`�F�C������o�b�t�@�擾��RTV�L�q�q�쐬�ɐ������܂����B");

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
            glib::Logger::FormatDebugLog("�t�F���X�J�E���^[%d]�̃��Z�b�g�ɐ������܂����B", i);
        }

        // �t�F���X����  
        hr = m_Device->CreateFence(m_FenceCounters[m_FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("D3D12�t�F���X�̍쐬�Ɏ��s���܂����B HRESULT=0x%08X", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("D3D12�t�F���X�̍쐬�ɐ������܂����B");

        m_FenceCounters[m_FrameIndex]++;

        // �C�x���g����  
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_FenceEvent == nullptr)
        {
            glib::Logger::FormatCriticalLog("�t�F���X�C�x���g�̍쐬�Ɏ��s���܂����B");
            return false;
        }
        glib::Logger::FormatDebugLog("�t�F���X�C�x���g�̍쐬�ɐ������܂����B");
    }

    // �R�}���h���X�g����
    m_CommandList->Close();

    return true;
}

void glib::GLibD3D12Wrapper::present(UINT32 interval)
{
    HRESULT hr{};
    hr = m_SwapChain->Present(interval, 0);

    if (FAILED(hr))
    {
        glib::Logger::FormatWarningLog("��ʕ`��iPresent�j�Ɏ��s���܂����B HRESULT=0x%08X", hr);
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
    WaitGpu();

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
    m_CommandList->OMSetRenderTargets(1, &m_HandlesRTV[m_FrameIndex], FALSE, &m_HandleDSV);

    // �����_�[�^�[�Q�b�g�r���[�̃N���A
    m_CommandList->ClearRenderTargetView(m_HandlesRTV[m_FrameIndex], &m_ClearColor.x, 0, nullptr);

    // �[�x�o�b�t�@�N���A
    m_CommandList->ClearDepthStencilView(m_HandleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // �`��
    {
        m_Rotate += XMConvertToRadians(45.0f) * glib::DeltaTime();
        m_CBVs[m_FrameIndex * 2 + 0].pBuffer->World = XMMatrixRotationZ(XMConvertToRadians(45.0f)) * XMMatrixRotationX(XMConvertToRadians(45.0f)) * XMMatrixRotationY(m_Rotate);
        m_CBVs[m_FrameIndex * 2 + 1].pBuffer->World = XMMatrixRotationY(m_Rotate) * XMMatrixScaling(2.0f, 0.9f, 0.9f);

        m_CommandList->SetGraphicsRootSignature(m_Rootsignature.Get());
        m_CommandList->SetDescriptorHeaps(1, m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetAddressOf());
        m_CommandList->SetGraphicsRootConstantBufferView(0, m_CBVs[m_FrameIndex].Desc.BufferLocation);
        m_CommandList->SetGraphicsRootDescriptorTable(1, m_Texture.HandleGPU);
        m_CommandList->SetPipelineState(m_PSO.Get());
        m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_CommandList->IASetVertexBuffers(0, 1, &m_VBV);
        m_CommandList->IASetIndexBuffer(&m_IBV);
        auto viewport = glib::GetViewport();
        m_CommandList->RSSetViewports(1, &viewport);
        auto rect = glib::GetRect();
        m_CommandList->RSSetScissorRects(1, &rect);

        auto count = static_cast<UINT32>(m_Meshes[0].Indices.size());
        m_CommandList->SetGraphicsRootConstantBufferView(0, m_CBVs[m_FrameIndex * 2 + 0].Desc.BufferLocation);
        m_CommandList->DrawIndexedInstanced(count, 1, 0, 0, 0);
    }
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

    WaitGpu();
}

void glib::GLibD3D12Wrapper::BeginRecordPerformance()
{
    m_RefreshTick = false;
    QueryPerformanceFrequency(&m_Freq);
    QueryPerformanceCounter(&m_Start);
}

void glib::GLibD3D12Wrapper::EndRecordPerformance()
{
    QueryPerformanceCounter(&m_End);
    m_RefreshTick = true;
}

void glib::GLibD3D12Wrapper::RunProfile()
{
    std::string title = glib::WStringToString(glib::GetWindow()->GetDefaultName());

    // �������g�p�ʎ擾
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

    SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

    // VRAM�g�p�ʎ擾
    SIZE_T vramUsed = 0;
    DXGI_QUERY_VIDEO_MEMORY_INFO vramInfo = {};
    GLibComPtr<IDXGIAdapter3> adapter;
    if (SUCCEEDED(m_Device->QueryInterface(IID_PPV_ARGS(&adapter))))
    {
        if (SUCCEEDED(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vramInfo)))
        {
            vramUsed = vramInfo.CurrentUsage / (1024 * 1024); // MB�P��
        }
    }

    std::string memoryInfo = "   |   [PhysMemUsed: " + std::to_string(physMemUsedByMe / (1024 * 1024)) + "MB";
    memoryInfo += " VirtMemUsed: " + std::to_string(virtualMemUsedByMe / (1024 * 1024)) + "MB";
    memoryInfo += " VRAMUsed: " + std::to_string(vramUsed) + "MB]";

    if (m_RefreshTick)
    {
        static int frame = 0;
        static float fps = 0.0f;
        if (++frame % 60 == 0)
        {
            float delta = static_cast<float>(DeltaTime());
            fps = 1.0f / delta;
        }

        float delta = static_cast<float>(DeltaTime());

        glib::GetWindow()->SetName(glib::StringToWString(
            title + "   |   [DeltaTime: " + std::to_string(delta) + "s]" +
            memoryInfo + "  |  [Performance: " + std::to_string(static_cast<int>(fps)) + "fps]").c_str());
    }
    else
    {
        glib::GetWindow()->SetName(glib::StringToWString(title + memoryInfo).c_str());
    }
}

void glib::GLibD3D12Wrapper::SetClearColor(const XMFLOAT4& color)
{
    m_ClearColor = color;
}

void glib::GLibD3D12Wrapper::WaitGpu()
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