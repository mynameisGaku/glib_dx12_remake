#pragma once
#include <d3d12.h>
#include <GLibComPtr.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibTexture
    {
    public:

    private:
        ComPtr<ID3D12Resource> m_TexcoordBuf;
        D3D12_VERTEX_BUFFER_VIEW m_TexcoordBufView;
    };
}