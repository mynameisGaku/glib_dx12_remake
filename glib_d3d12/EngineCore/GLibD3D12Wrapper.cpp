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

    // ディスクリプタヒープの生成
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 5000;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;

        hr = m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("ディスクリプタヒープの作成に失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("ディスクリプタヒープの作成に成功しました。");
    }

    // テクスチャ生成
    {
        ResourceUploadBatch batch(m_Device.Get());
        batch.Begin();

        // 生成
        std::wstring path = L"Resources/Models/teapot/default.DDS";
        hr = CreateDDSTextureFromFile(m_Device.Get(), batch, path.c_str(), m_Texture.Resource.GetAddressOf(), true);
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("DDSファイルの読み込みに失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("DDSファイルの読み込みに成功しました。");

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

        // SRVの設定
        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
        viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        viewDesc.Format = texDesc.Format;
        viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        viewDesc.Texture2D.MostDetailedMip = 0;
        viewDesc.Texture2D.MipLevels = texDesc.MipLevels;
        viewDesc.Texture2D.PlaneSlice = 0;
        viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        // SRV生成
        m_Device->CreateShaderResourceView(m_Texture.Resource.Get(), &viewDesc, handleCPU);

    }

    // メッシュロード
    {
        std::wstring path = L"Resources/Models/teapot/teapot.obj";
        if (not LoadMesh(path.c_str(), m_Meshes, m_Materials))
        {
            glib::Logger::FormatErrorLog("メッシュの読み込みに失敗しました。file: %s", path.c_str());
            return false;
        }
        glib::Logger::FormatDebugLog("メッシュの読み込みに成功しました。file: %s", path.c_str());
    }

    // 頂点バッファ
    {
        auto& vertices = m_Meshes[0].Vertices;
        auto vbSize = sizeof(GLibMeshVertex) * vertices.size();

        // ヒープ設定
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;

        // リソース設定
        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width = vbSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        // リソース作成
        hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_VB.GetAddressOf()));
        assert(SUCCEEDED(hr));

        // データ転送
        void* ptr = nullptr;
        m_VB->Map(0, nullptr, &ptr);
        memcpy(ptr, vertices.data(), vbSize);
        m_VB->Unmap(0, nullptr);

        // VBV設定
        m_VBV.BufferLocation = m_VB->GetGPUVirtualAddress();
        m_VBV.SizeInBytes = static_cast<UINT>(vbSize);
        m_VBV.StrideInBytes = sizeof(GLibMeshVertex);
    }


    // インデックスバッファ
    {

        // インデックスデータ  
        auto size = sizeof(UINT32) * m_Meshes[0].Indices.size();
        auto indices = m_Meshes[0].Indices.data();

        // ヒーププロパティ
        D3D12_HEAP_PROPERTIES prop{};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // リソース設定
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

        // リソース生成
        hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_IB.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("リソースの生成に失敗しました。HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("リソースの生成に成功しました。");

        // マップ
        void* ptr = nullptr;
        hr = m_IB->Map(0, nullptr, &ptr);
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("リソースのマップに失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("リソースのマップに成功しました。");

        // 頂点データをマップ先に
        memcpy(ptr, indices, size);

        // マップ解除
        m_IB->Unmap(0, nullptr);

        // 頂点バッファビューの設定
        m_IBV.BufferLocation = m_IB->GetGPUVirtualAddress();
        m_IBV.Format = DXGI_FORMAT_R32_UINT;
        m_IBV.SizeInBytes = static_cast<UINT>(size);
    }

    // 定数バッファ生成
    {
        D3D12_HEAP_PROPERTIES prop{};
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        // 設定
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
            // リソース生成
            hr = m_Device->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_CB[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("リソースの作成に失敗しました。 HRESULT=0x%x", hr);
                return false;
            }
            glib::Logger::FormatDebugLog("リソースの作成に成功しました。");

            auto address = m_CB[i]->GetGPUVirtualAddress();
            auto handleCPU = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetCPUDescriptorHandleForHeapStart();
            auto handleGPU = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->GetGPUDescriptorHandleForHeapStart();

            handleCPU.ptr += incrementSize * i;
            handleGPU.ptr += incrementSize * i;

            // 定数バッファビュー設定
            m_CBVs[i].HandleCPU = handleCPU;
            m_CBVs[i].HandleGPU = handleGPU;
            m_CBVs[i].Desc.BufferLocation = address;
            m_CBVs[i].Desc.SizeInBytes = sizeof(Transform);

            // 定数バッファビュー生成
            m_Device->CreateConstantBufferView(&m_CBVs[i].Desc, handleCPU);

            // マッピング
            hr = m_CB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBVs[i].pBuffer));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("リソースのマッピングに失敗しました。 HRESULT=0x%x", hr);
                return false;
            }
            glib::Logger::FormatDebugLog("リソースのマッピングに成功しました。");

            auto eyePos = XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
            auto targetPos = XMVectorZero();
            auto upward = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

            auto fovY = XMConvertToRadians(37.5f);
            auto aspect = static_cast<float>(glib::GetWindow()->GetClientWidth()) / static_cast<float>(glib::GetWindow()->GetClientHeight());

            // 変換行列
            m_CBVs[i].pBuffer->World = XMMatrixIdentity();
            m_CBVs[i].pBuffer->View = XMMatrixLookAtRH(eyePos, targetPos, upward);
            m_CBVs[i].pBuffer->Proj = XMMatrixPerspectiveFovRH(fovY, aspect, 0.01f, 100000.0f);
        }
    }

    // 深度バッファ
    {
        // リソース
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
            glib::Logger::FormatCriticalLog("深度バッファリソースの生成に失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("深度バッファリソースの生成に成功しました。");

        // ディスクリプタヒープ
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.NumDescriptors = 1;
        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        heapDesc.NodeMask = 0;

        hr = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_DSV].GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("深度バッファディスクリプタヒープの生成に失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("深度バッファディスクリプタヒープ生成に成功しました。");

        auto handle = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_DSV]->GetCPUDescriptorHandleForHeapStart();
        auto incrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        
        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc{};
        viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;
        viewDesc.Flags = D3D12_DSV_FLAG_NONE;

        // 生成
        m_Device->CreateDepthStencilView(m_DepthBuffer.Get(), &viewDesc, handle);

        m_HandleDSV = handle;
    }

    // ルートシグネチャ生成
    {
        auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
        flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
        flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
        flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        // ルートパラメーター
        D3D12_ROOT_PARAMETER param[2]{};
        param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        param[0].Descriptor.ShaderRegister = 0;
        param[0].Descriptor.RegisterSpace = 0;
        param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        // レンジ
        D3D12_DESCRIPTOR_RANGE range{};
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors = 5000;
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.OffsetInDescriptorsFromTableStart = 0;
        
        // パラメーター2
        param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        param[1].DescriptorTable.NumDescriptorRanges = 1;
        param[1].DescriptorTable.pDescriptorRanges = &range;
        param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // スタティックサンプラ
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

        // ルートシグネチャ
        D3D12_ROOT_SIGNATURE_DESC desc{};
        desc.NumParameters = 2;
        desc.NumStaticSamplers = 1;
        desc.pParameters = param;
        desc.pStaticSamplers = &sampler;
        desc.Flags = flag;

        GLibComPtr<ID3DBlob> Blob = nullptr;
        GLibComPtr<ID3DBlob> ErrorBlob = nullptr;

        // シリアライズ
        hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, Blob.GetAddressOf(), ErrorBlob.GetAddressOf());
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("ルートシグネチャのシリアライズに失敗しました。HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("ルートシグネチャのシリアライズに成功しました。");

        // ルートシグネチャ生成
        hr = m_Device->CreateRootSignature(0, Blob->GetBufferPointer(), Blob->GetBufferSize(), IID_PPV_ARGS(m_Rootsignature.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("ルートシグネチャの生成に失敗しました。HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("ルートシグネチャの生成に成功しました。");
    }

    // パイプラインステート生成
    {
        // ラスタライザーステートの設定
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
        
        // レンダーターゲットのブレンド設定
        D3D12_RENDER_TARGET_BLEND_DESC descRTBS =
        {
            FALSE, FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL
        };

        // ブレンドステートの設定
        D3D12_BLEND_DESC descBS{};
        descBS.AlphaToCoverageEnable = FALSE;
        descBS.IndependentBlendEnable = FALSE;
        for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
        {
            descBS.RenderTarget[i] = descRTBS;
        }

        // ここでやっている複数の設定(DESC)は全て、一つのパイプラインの設定をしています。
        // 最終的にまとめてようやく、一つのパイプラインが出来上がります。
        // 細かな設定は、パイプラインの種類によって異なるので、いろいろ調べつつ、自分が求めているパイプラインにカスタムしよう。
#ifdef _DEBUG
        glib::GLibBinaryLoader vs("x64/Debug/SpriteVS.cso");
        glib::GLibBinaryLoader ps("x64/Debug/SpritePS.cso");
#else
        glib::GLibBinaryLoader vs("x64/Release/SpriteVS.cso");
        glib::GLibBinaryLoader ps("x64/Release/SpritePS.cso");
#endif

        // 深度ステンシルステートの設定
        D3D12_DEPTH_STENCIL_DESC descDSS{};
        descDSS.DepthEnable = TRUE;
        descDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        descDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        descDSS.StencilEnable = FALSE;

        // パイプラインステートの設定
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

        // パイプラインステート生成
        hr = m_Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_PSO.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("パイプラインステートの作成に失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("パイプラインステートの作成に成功しました。");
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
    デバイス
    ↓
    コマンドキュー
    ↓
    スワップチェイン
    ↓
    コマンドアロケータ
    ↓
    コマンドリスト
    ↓
    レンダーターゲットビュー
    ↓
    フェンス
    */

    HRESULT hr{};

    // Enable the debug layer
    glib::GLibDebug::EnableDebugLayer();

    // デバイス
    {
        hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_Device.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("D3D12デバイスの作成に失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("D3D12デバイスの作成に成功しました。");
    }

    // コマンドキュー
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;
        hr = m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("D3D12コマンドキューの作成に失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("D3D12コマンドキューの作成に成功しました。");
    }

    // スワップチェイン
    {
        // DXGIファクトリーの生成
        GLibComPtr<IDXGIFactory4> factory{};
        hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("DXGIファクトリの作成に失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("DXGIファクトリの作成に成功しました。");

        // スワップチェインの設定
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

        // スワップチェインの生成
        IDXGISwapChain* swapChain{};

        hr = factory->CreateSwapChain(m_CommandQueue.Get(), &desc, &swapChain);
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("DXGIスワップチェインの作成に失敗しました。 HRESULT=0x%x", hr);
            factory.Reset();
            return false;
        }
        glib::Logger::FormatDebugLog("DXGIスワップチェインの作成に成功しました。");

        // IDXGISwapChain3を取得
        hr = swapChain->QueryInterface(IID_PPV_ARGS(m_SwapChain.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("DXGIスワップチェイン3の作成に失敗しました。 HRESULT=0x%x", hr);
            factory.Reset();
            glib::SafeReleaseDX(swapChain);
            return false;
        }
        glib::Logger::FormatDebugLog("DXGIスワップチェイン3の作成に成功しました。");

        // バックバッファ番号を取得
        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        // 不要なので削除
        factory.Reset();
        glib::SafeReleaseDX(swapChain);
    }

    // コマンドアロケーター
    {
        for (UINT32 i = 0; i < m_FrameCount; i++)
        {
            m_CommandAllocators.push_back(GLibComPtr<ID3D12CommandAllocator>());
            hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocators[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("D3D12コマンドアロケータの作成に失敗しました。 [%d]. HRESULT=0x%x", i, hr);
                return false;
            }
            glib::Logger::FormatDebugLog("D3D12コマンドアロケータの作成に成功しました。");
        }
    }

    // コマンドリスト
    {
        hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[m_FrameIndex].Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("D3D12コマンドリストの作成に失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("D3D12コマンドリストの作成に成功しました。");
    }

    // レンダーターゲットビュー
    {
        // ディスクリプタヒープの設定
        D3D12_DESCRIPTOR_HEAP_DESC desc{};

        desc.NumDescriptors = m_FrameCount;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 0;

        for (int i = 0; i < GLIB_DESCRIPTOR_HEAP_TYPE_MAX; i++)
        {
            m_DescriptorHeaps.push_back(GLibComPtr<ID3D12DescriptorHeap>());
        }

        // ディスクリプタヒープ生成
        hr = m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_RTV].GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("D3D12 RTVディスクリプタヒープの作成に失敗しました。 HRESULT=0x%x", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("D3D12 RTVディスクリプタヒープの作成に成功しました。");

        auto handle = m_DescriptorHeaps[GLIB_DESCRIPTOR_HEAP_TYPE_RTV]->GetCPUDescriptorHandleForHeapStart();
        auto incrementSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (auto i = 0u; i < m_FrameCount; i++)
        {
            m_ColorBuffers.push_back(GLibComPtr<ID3D12Resource>());
            hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_ColorBuffers[i].GetAddressOf()));
            if (FAILED(hr))
            {
                glib::Logger::FormatCriticalLog("スワップチェインからのバッファ取得に失敗しました（インデックス [%d]. HRESULT=0x%08X", i, hr);
                return false;
            }
            glib::Logger::FormatDebugLog("スワップチェインからバッファ取得とRTV記述子作成に成功しました。");

            D3D12_RENDER_TARGET_VIEW_DESC viewDesc{};
            viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipSlice = 0;
            viewDesc.Texture2D.PlaneSlice = 0;

            // レンダーターゲットビュー生成
            m_Device->CreateRenderTargetView(m_ColorBuffers[i].Get(), &viewDesc, handle);

            m_HandlesRTV.push_back(D3D12_CPU_DESCRIPTOR_HANDLE());
            m_HandlesRTV[i] = handle;
            handle.ptr += incrementSize;
        }
    }

    // フェンス
    {
        // カウンターリセット
        for (auto i = 0u; i < m_FrameCount; i++)
        {
            m_FenceCounters.push_back(UINT64());
            m_FenceCounters[i] = 0;
            glib::Logger::FormatDebugLog("フェンスカウンタ[%d]のリセットに成功しました。", i);
        }

        // フェンス生成  
        hr = m_Device->CreateFence(m_FenceCounters[m_FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));
        if (FAILED(hr))
        {
            glib::Logger::FormatCriticalLog("D3D12フェンスの作成に失敗しました。 HRESULT=0x%08X", hr);
            return false;
        }
        glib::Logger::FormatDebugLog("D3D12フェンスの作成に成功しました。");

        m_FenceCounters[m_FrameIndex]++;

        // イベント生成  
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_FenceEvent == nullptr)
        {
            glib::Logger::FormatCriticalLog("フェンスイベントの作成に失敗しました。");
            return false;
        }
        glib::Logger::FormatDebugLog("フェンスイベントの作成に成功しました。");
    }

    // コマンドリスト閉じる
    m_CommandList->Close();

    return true;
}

void glib::GLibD3D12Wrapper::present(UINT32 interval)
{
    HRESULT hr{};
    hr = m_SwapChain->Present(interval, 0);

    if (FAILED(hr))
    {
        glib::Logger::FormatWarningLog("画面描画（Present）に失敗しました。 HRESULT=0x%08X", hr);
    }

    // シグナル
    const auto currentValue = m_FenceCounters[m_FrameIndex];
    m_CommandQueue->Signal(m_Fence.Get(), currentValue);

    // バックバッファ番号更新
    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

    // 次のフレームの描画準備がまだなら待機
    if (m_Fence->GetCompletedValue() < m_FenceCounters[m_FrameIndex])
    {
        m_Fence->SetEventOnCompletion(m_FenceCounters[m_FrameIndex], m_FenceEvent);
        WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);
    }

    // 次のフレームのフェンスカウンター増やす
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
    // コマンド記録開始
    m_CommandAllocators[m_FrameIndex]->Reset();
    m_CommandList->Reset(m_CommandAllocators[m_FrameIndex].Get(), nullptr);

    // リソースバリア（Present → RenderTarget）
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_ColorBuffers[m_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // リソースバリア
    m_CommandList->ResourceBarrier(1, &barrier);

    // レンダーターゲット設定
    m_CommandList->OMSetRenderTargets(1, &m_HandlesRTV[m_FrameIndex], FALSE, &m_HandleDSV);

    // レンダーターゲットビューのクリア
    m_CommandList->ClearRenderTargetView(m_HandlesRTV[m_FrameIndex], &m_ClearColor.x, 0, nullptr);

    // 深度バッファクリア
    m_CommandList->ClearDepthStencilView(m_HandleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 描画
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
    // リソースバリア（RenderTarget → Present）
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_ColorBuffers[m_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_CommandList->ResourceBarrier(1, &barrier);

    // コマンドリストの記録終了
    m_CommandList->Close();

    // 実行
    ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(1, ppCommandLists);

    // Present
    present(1);

    // フェンス同期
    m_CommandQueue->Signal(m_Fence.Get(), m_FenceCounters[m_FrameIndex]);
    m_FenceCounters[m_FrameIndex]++;

    // 次フレームへ
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

    // メモリ使用量取得
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

    SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

    // VRAM使用量取得
    SIZE_T vramUsed = 0;
    DXGI_QUERY_VIDEO_MEMORY_INFO vramInfo = {};
    GLibComPtr<IDXGIAdapter3> adapter;
    if (SUCCEEDED(m_Device->QueryInterface(IID_PPV_ARGS(&adapter))))
    {
        if (SUCCEEDED(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vramInfo)))
        {
            vramUsed = vramInfo.CurrentUsage / (1024 * 1024); // MB単位
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

    // シグナル
    m_CommandQueue->Signal(m_Fence.Get(), m_FenceCounters[m_FrameIndex]);

    // 完了時にイベントセット
    m_Fence->SetEventOnCompletion(m_FenceCounters[m_FrameIndex], m_FenceEvent);

    // 待機処理
    WaitForSingleObjectEx(m_FenceEvent, INFINITE, FALSE);

    // カウンターフヤス
    m_FenceCounters[m_FrameIndex]++;
}