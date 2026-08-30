#include "DX11Renderer.h"

#include <cassert>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

bool DX11Renderer::Initialize(
    HWND hwnd,
    int width,
    int height)
{
    if (!CreateDeviceAndSwapChain(
        hwnd,
        width,
        height))
    {
        return false;
    }

    if (!CreateRenderTarget())
    {
        return false;
    }

    D3D11_VIEWPORT viewport{};

    viewport.Width =
        static_cast<float>(width);

    viewport.Height =
        static_cast<float>(height);

    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(
        1,
        &viewport
    );

    return true;
}

bool DX11Renderer::CreateDeviceAndSwapChain(
    HWND hwnd,
    int width,
    int height)
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};

    swapChainDesc.BufferCount = 2;

    swapChainDesc.BufferDesc.Width =
        static_cast<UINT>(width);

    swapChainDesc.BufferDesc.Height =
        static_cast<UINT>(height);

    swapChainDesc.BufferDesc.Format =
        DXGI_FORMAT_R8G8B8A8_UNORM;

    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

    swapChainDesc.BufferUsage =
        DXGI_USAGE_RENDER_TARGET_OUTPUT;

    swapChainDesc.OutputWindow = hwnd;

    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    swapChainDesc.Windowed = TRUE;

    swapChainDesc.SwapEffect =
        DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT creationFlags = 0;

#ifdef _DEBUG
    // 개발 중에는 Direct3D 디버그 레이어를
    // 사용할 수 있다.
    creationFlags |=
        D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };

    D3D_FEATURE_LEVEL featureLevel{};

    HRESULT hr =
        D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &swapChainDesc,
            m_swapChain.GetAddressOf(),
            m_device.GetAddressOf(),
            &featureLevel,
            m_context.GetAddressOf()
        );

    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

bool DX11Renderer::CreateRenderTarget()
{
    Microsoft::WRL::ComPtr<
        ID3D11Texture2D
    > backBuffer;

    HRESULT hr =
        m_swapChain->GetBuffer(
            0,
            IID_PPV_ARGS(
                backBuffer.GetAddressOf()
            )
        );

    if (FAILED(hr))
    {
        return false;
    }

    hr =
        m_device->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            m_renderTargetView.GetAddressOf()
        );

    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void DX11Renderer::BeginFrame()
{
    m_context->OMSetRenderTargets(
        1,
        m_renderTargetView.GetAddressOf(),
        nullptr
    );

    Clear();
}

void DX11Renderer::Clear()
{
    m_context->ClearRenderTargetView(
        m_renderTargetView.Get(),
        m_clearColor
    );
}

void DX11Renderer::EndFrame()
{
    m_swapChain->Present(
        1,
        0
    );
}