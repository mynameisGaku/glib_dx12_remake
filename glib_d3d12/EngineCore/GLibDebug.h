#pragma once
#include <d3d12.h>
#include <GLibLogger.h>

namespace glib
{
    class GLibDebug
    {
    public:
        static void EnableDebugLayer()
        {
            // D3D12デバッグレイヤーを有効にする
            ID3D12Debug* debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                debugController->Release();
                Logger::DebugLog("D3D12 debug layer enabled successfully.");
            }
            else
            {
                Logger::ErrorLog("Failed to enable D3D12 debug layer.");
            }
        }
    };
}