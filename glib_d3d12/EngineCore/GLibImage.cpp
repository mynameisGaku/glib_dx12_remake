#include <GLibImage.h>
#include <GLibLogger.h>

glib::GLibImage::GLibImage()
{
}

glib::GLibImage::~GLibImage()
{
    if (m_pDiffuseBuf)
    {
        delete m_pDiffuseBuf;
        m_pDiffuseBuf = nullptr;
    }

    if (m_pPositionBuf)
    {
        delete m_pPositionBuf;
        m_pPositionBuf = nullptr;
    }

    if (m_pTextureBuf)
    {
        delete m_pTextureBuf;
        m_pTextureBuf = nullptr;
    }
}

bool glib::GLibImage::Initialize(GLibDevice* pDevice, GLibDescriptorPool* pPool, GLibGraphicsCommandList* pCmdList, const std::string& filepath)
{
    m_pDevice = pDevice;
    if (not m_pDevice)
    {
        glib::Logger::ErrorLog("GLibImage initialize failed. Cuz device is nullptr.");
        return false;
    }

    m_pCmdList = pCmdList;
    if (not m_pCmdList)
    {
        glib::Logger::ErrorLog("GLibImage initialize failed. Cuz command list is nullptr.");
        return false;
    }

    m_pDescriptorPool = pPool;
    if (not m_pDescriptorPool)
    {
        glib::Logger::ErrorLog("GLibImage initialize failed. Cuz descriptor pool is nullptr.");
        return false;
    }

    glib::Logger::DebugLog("GLibImage is Initializing the Heap...");
    auto resHeap = m_pDescriptorPool->Get(GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    auto resHeapSize = m_pDevice->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // cbv記述
    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Alignment = 0;
    resDesc.Width = 256;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1; // No multisampling
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // Row-major layout
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE; // No special flags for constant buffers

    // cbvつくる
    m_pPositionBuf = new GLibConstantBuffer();
    if (not m_pPositionBuf->Initialize(m_pDevice, m_pDescriptorPool, resDesc))
    {
        glib::Logger::ErrorLog("GLibImage is Initialize failed. Cuz initialize failed the m_pPositionBuf.");
    }

    m_pDiffuseBuf = new GLibConstantBuffer();
    if (not m_pDiffuseBuf->Initialize(m_pDevice, m_pDescriptorPool, resDesc))
    {
        glib::Logger::ErrorLog("GLibImage is Initialize failed. Cuz initialize failed the m_pDiffuseBuf.");
    }

    // テクスチャ読む
    m_pTextureBuf = new GLibTexture();
    if (not m_pTextureBuf->Initialize(m_pDevice, m_pDescriptorPool, filepath))
    {
        glib::Logger::ErrorLog("GLibImage is Initialize failed. Cuz initialize failed the m_pTextureBuf.");
    }

    auto pHeap = m_pDescriptorPool->Get(GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    auto hHeap = pHeap->Get()->GetCPUDescriptorHandleForHeapStart();
    auto increment = pHeap->GetIncrementSize();

    // ヒープにビューを作る

    // 位置
    D3D12_CONSTANT_BUFFER_VIEW_DESC posViewDesc = {};
    posViewDesc.BufferLocation = m_pPositionBuf->GetResource()->GetGPUVirtualAddress();
    posViewDesc.SizeInBytes = static_cast<UINT>(m_pPositionBuf->GetResource()->GetDesc().Width);
    m_pDevice->Get()->CreateConstantBufferView(&posViewDesc, hHeap);
    m_hPositionHeap = hHeap;
    m_PositionHeapIndex = pHeap->GetIndex();

    hHeap.ptr += increment;
    pHeap->AddIndex();

    // ディフューズ
    m_hDiffuseHeap = hHeap;
    D3D12_CONSTANT_BUFFER_VIEW_DESC diffuseViewDesc = {};
    diffuseViewDesc.BufferLocation = m_pDiffuseBuf->GetResource()->GetGPUVirtualAddress();
    diffuseViewDesc.SizeInBytes = static_cast<UINT>(m_pPositionBuf->GetResource()->GetDesc().Width);
    m_pDevice->Get()->CreateConstantBufferView(&diffuseViewDesc, hHeap);
    m_hDiffuseHeap = hHeap;
    m_DiffuseHeapIndex = pHeap->GetIndex();

    hHeap.ptr += increment;
    pHeap->AddIndex();

    // シェーダーリソース(テクスチャ)
    m_hTextureHeap = hHeap;
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = m_pTextureBuf->Get()->GetDesc().Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    m_pDevice->Get()->CreateShaderResourceView(m_pTextureBuf->Get(), &srvDesc, hHeap);
    m_hTextureHeap = hHeap;
    m_TextureHeapIndex = pHeap->GetIndex();

    pHeap->AddIndex();

    return true;
}

void glib::GLibImage::Draw()
{
    auto resHeap = m_pDescriptorPool->Get(GLIB_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    auto directx_heap = resHeap->Get();
    m_pCmdList->Get()->SetDescriptorHeaps(1, &directx_heap);
    auto hHeap = directx_heap->GetGPUDescriptorHandleForHeapStart();
    m_pCmdList->Get()->SetGraphicsRootDescriptorTable(0, hHeap);

    m_pCmdList->Get()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
