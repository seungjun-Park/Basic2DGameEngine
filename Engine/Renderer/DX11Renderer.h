#pragma once

#include "IRenderer.h"

#include <d3d11.h>
#include <dxgi.h>

#include <wrl/client.h>

class DX11Renderer : public IRenderer
{
public:
    DX11Renderer() = default;
    ~DX11Renderer() override = default;

    DX11Renderer(const DX11Renderer&) = delete;
    DX11Renderer& operator=(const DX11Renderer&) = delete;

    bool Initialize(
        HWND hwnd,
        int width,
        int height
    ) override;

    void BeginFrame() override;

    void EndFrame() override;

    void Clear() override;

    ID3D11Device* GetDevice() const
    {
        return m_device.Get();
    }

    ID3D11DeviceContext* GetContext() const
    {
        return m_context.Get();
    }

private:
    bool CreateDeviceAndSwapChain(
        HWND hwnd,
        int width,
        int height
    );

    bool CreateRenderTarget();

private:
    Microsoft::WRL::ComPtr<
        ID3D11Device
    > m_device;

    Microsoft::WRL::ComPtr<
        ID3D11DeviceContext
    > m_context;

    Microsoft::WRL::ComPtr<
        IDXGISwapChain
    > m_swapChain;

    Microsoft::WRL::ComPtr<
        ID3D11RenderTargetView
    > m_renderTargetView;

    float m_clearColor[4]
    {
        0.08f,
        0.08f,
        0.12f,
        1.0f
    };
};