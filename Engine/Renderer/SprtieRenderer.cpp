#include "SpriteRenderer.h"
#include "Camera.h"

#include "DX11Renderer.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Components/Sprite.h"

#include <d3dcompiler.h>

#include <vector>
#include <fstream>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3dcompiler.lib")

namespace
{
    struct SpriteVertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    struct SpriteConstantBuffer
    {
        XMMATRIX world;
        XMMATRIX view;
        XMMATRIX projection;
    };
}

bool SpriteRenderer::Initialize(
    DX11Renderer& renderer,
    int screenWidth,
    int screenHeight)
{
    m_renderer = &renderer;

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // 2D 화면에서는 카메라를 따로 두지 않고
    // 기본적으로 화면 전체를 바라보도록 한다.
    m_viewMatrix =
        XMMatrixIdentity();

    m_projectionMatrix =
        XMMatrixOrthographicOffCenterLH(
            0.0f,
            static_cast<float>(screenWidth),
            static_cast<float>(screenHeight),
            0.0f,
            0.0f,
            1.0f
        );

    if (!CreateShaders())
        return false;

    if (!CreateInputLayout())
        return false;

    if (!CreateBuffers())
        return false;

    if (!CreateSampler())
        return false;

    if (!CreateBlendStates())
        return false;

    return true;
}

bool SpriteRenderer::CompileShader(
    const wchar_t* filename,
    const char* entryPoint,
    const char* target,
    ComPtr<ID3DBlob>& blob)
{
    UINT flags =
        D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
    flags |=
        D3DCOMPILE_DEBUG;

    flags |=
        D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr =
        D3DCompileFromFile(
            filename,
            nullptr,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint,
            target,
            flags,
            0,
            blob.GetAddressOf(),
            errorBlob.GetAddressOf()
        );

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA(
                static_cast<const char*>(
                    errorBlob->GetBufferPointer()
                    )
            );
        }

        return false;
    }

    return true;
}

bool SpriteRenderer::CreateShaders()
{
    ComPtr<ID3DBlob> vertexShaderBlob;
    ComPtr<ID3DBlob> pixelShaderBlob;

    if (!CompileShader(
        L"Engine/Assets/Shaders/SpriteVS.hlsl",
        "main",
        "vs_5_0",
        vertexShaderBlob))
    {
        return false;
    }

    if (!CompileShader(
        L"Engine/Assets/Shaders/SpritePS.hlsl",
        "main",
        "ps_5_0",
        pixelShaderBlob))
    {
        return false;
    }

    ID3D11Device* device =
        m_renderer->GetDevice();

    HRESULT hr =
        device->CreateVertexShader(
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            nullptr,
            m_vertexShader.GetAddressOf()
        );

    if (FAILED(hr))
        return false;

    hr =
        device->CreatePixelShader(
            pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize(),
            nullptr,
            m_pixelShader.GetAddressOf()
        );

    if (FAILED(hr))
        return false;

    return true;
}

bool SpriteRenderer::CreateInputLayout()
{
    ComPtr<ID3DBlob> vertexShaderBlob;

    if (!CompileShader(
        L"Engine/Assets/Shaders/SpriteVS.hlsl",
        "main",
        "vs_5_0",
        vertexShaderBlob))
    {
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },

        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            sizeof(float) * 3,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    HRESULT hr =
        m_renderer->GetDevice()->CreateInputLayout(
            layout,
            ARRAYSIZE(layout),
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            m_inputLayout.GetAddressOf()
        );

    return SUCCEEDED(hr);
}

bool SpriteRenderer::CreateBuffers()
{
    SpriteVertex vertices[] =
    {
        // position               UV
        { { 0.0f, 0.0f, 0.0f },   { 0.0f, 0.0f } },
        { { 1.0f, 0.0f, 0.0f },   { 1.0f, 0.0f } },
        { { 0.0f, 1.0f, 0.0f },   { 0.0f, 1.0f } },
        { { 1.0f, 1.0f, 0.0f },   { 1.0f, 1.0f } }
    };

    D3D11_BUFFER_DESC vertexBufferDesc{};

    vertexBufferDesc.Usage =
        D3D11_USAGE_DEFAULT;

    vertexBufferDesc.ByteWidth =
        sizeof(vertices);

    vertexBufferDesc.BindFlags =
        D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData{};

    vertexData.pSysMem =
        vertices;

    HRESULT hr =
        m_renderer->GetDevice()->CreateBuffer(
            &vertexBufferDesc,
            &vertexData,
            m_vertexBuffer.GetAddressOf()
        );

    if (FAILED(hr))
        return false;

    unsigned int indices[] =
    {
        0, 1, 2,
        2, 1, 3
    };

    D3D11_BUFFER_DESC indexBufferDesc{};

    indexBufferDesc.Usage =
        D3D11_USAGE_DEFAULT;

    indexBufferDesc.ByteWidth =
        sizeof(indices);

    indexBufferDesc.BindFlags =
        D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData{};

    indexData.pSysMem =
        indices;

    hr =
        m_renderer->GetDevice()->CreateBuffer(
            &indexBufferDesc,
            &indexData,
            m_indexBuffer.GetAddressOf()
        );

    if (FAILED(hr))
        return false;

    // Constant Buffer
    D3D11_BUFFER_DESC constantBufferDesc{};

    constantBufferDesc.Usage =
        D3D11_USAGE_DYNAMIC;

    constantBufferDesc.ByteWidth =
        sizeof(SpriteConstantBuffer);

    constantBufferDesc.BindFlags =
        D3D11_BIND_CONSTANT_BUFFER;

    constantBufferDesc.CPUAccessFlags =
        D3D11_CPU_ACCESS_WRITE;

    hr =
        m_renderer->GetDevice()->CreateBuffer(
            &constantBufferDesc,
            nullptr,
            m_constantBuffer.GetAddressOf()
        );

    if (FAILED(hr))
        return false;

    return true;
}

bool SpriteRenderer::CreateSampler()
{
    D3D11_SAMPLER_DESC samplerDesc{};

    samplerDesc.Filter =
        D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    samplerDesc.AddressU =
        D3D11_TEXTURE_ADDRESS_CLAMP;

    samplerDesc.AddressV =
        D3D11_TEXTURE_ADDRESS_CLAMP;

    samplerDesc.AddressW =
        D3D11_TEXTURE_ADDRESS_CLAMP;

    samplerDesc.ComparisonFunc =
        D3D11_COMPARISON_NEVER;

    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD =
        D3D11_FLOAT32_MAX;

    HRESULT hr =
        m_renderer->GetDevice()->CreateSamplerState(
            &samplerDesc,
            m_samplerState.GetAddressOf()
        );

    return SUCCEEDED(hr);
}

void SpriteRenderer::Begin()
{
    m_drawCallCount = 0;

    auto* context =
        m_renderer->GetContext();

    context->IASetInputLayout(
        m_inputLayout.Get()
    );

    context->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    );

    UINT stride =
        sizeof(SpriteVertex);

    UINT offset = 0;

    context->IASetVertexBuffers(
        0,
        1,
        m_vertexBuffer.GetAddressOf(),
        &stride,
        &offset
    );

    context->IASetIndexBuffer(
        m_indexBuffer.Get(),
        DXGI_FORMAT_R32_UINT,
        0
    );

    context->VSSetShader(
        m_vertexShader.Get(),
        nullptr,
        0
    );

    context->PSSetShader(
        m_pixelShader.Get(),
        nullptr,
        0
    );

    context->PSSetSamplers(
        0,
        1,
        m_samplerState.GetAddressOf()
    );

    m_currentBlendMode =
        BlendMode::Alpha;

    float blendFactor[4] =
    {
        0.0f,
        0.0f,
        0.0f,
        0.0f
    };

    context->OMSetBlendState(
        m_alphaBlendState.Get(),
        blendFactor,
        0xffffffff
    );
}

void SpriteRenderer::End()
{
}

void SpriteRenderer::Draw(
    const Sprite& sprite,
    const Transform& transform)
{
    auto* context =
        m_renderer->GetContext();

    // 좌상단 중심
    /*XMMATRIX world =
        XMMatrixScaling(
            transform.scale.x,
            transform.scale.y,
            1.0f
        )
        *
        XMMatrixRotationZ(
            transform.rotation
        )
        *
        XMMatrixTranslation(
            transform.position.x,
            transform.position.y,
            0.0f
        );*/
    // 중심
    XMMATRIX world =
        XMMatrixScaling(
            transform.scale.x,
            transform.scale.y,
            1.0f
        )
        *
        XMMatrixTranslation(
            -0.5f,
            -0.5f,
            0.0f
        )
        *
        XMMatrixRotationZ(
            transform.rotation
        )
        *
        XMMatrixTranslation(
            transform.position.x,
            transform.position.y,
            0.0f
        );

    SpriteConstantBuffer buffer{};

    buffer.world =
        XMMatrixTranspose(world);

    buffer.view =
        XMMatrixTranspose(
            m_viewMatrix
        );

    buffer.projection =
        XMMatrixTranspose(
            m_projectionMatrix
        );

    D3D11_MAPPED_SUBRESOURCE mapped{};

    HRESULT hr =
        context->Map(
            m_constantBuffer.Get(),
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mapped
        );

    if (FAILED(hr))
        return;

    memcpy(
        mapped.pData,
        &buffer,
        sizeof(buffer)
    );

    context->Unmap(
        m_constantBuffer.Get(),
        0
    );

    context->VSSetConstantBuffers(
        0,
        1,
        m_constantBuffer.GetAddressOf()
    );

    if (!sprite.texture)
        return;

    ID3D11ShaderResourceView* srv =
        sprite.texture->GetSRV();

    context->PSSetShaderResources(
        0,
        1,
        &srv
    );

    context->DrawIndexed(
        6,
        0,
        0
    );

    ++m_drawCallCount;
}

bool SpriteRenderer::CreateBlendStates()
{
    D3D11_BLEND_DESC alphaDesc{};

    alphaDesc.RenderTarget[0].BlendEnable =
        TRUE;

    alphaDesc.RenderTarget[0].SrcBlend =
        D3D11_BLEND_SRC_ALPHA;

    alphaDesc.RenderTarget[0].DestBlend =
        D3D11_BLEND_INV_SRC_ALPHA;

    alphaDesc.RenderTarget[0].BlendOp =
        D3D11_BLEND_OP_ADD;

    alphaDesc.RenderTarget[0].SrcBlendAlpha =
        D3D11_BLEND_ONE;

    alphaDesc.RenderTarget[0].DestBlendAlpha =
        D3D11_BLEND_INV_SRC_ALPHA;

    alphaDesc.RenderTarget[0].BlendOpAlpha =
        D3D11_BLEND_OP_ADD;

    alphaDesc.RenderTarget[0].
        RenderTargetWriteMask =
        D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr =
        m_renderer->GetDevice()->
        CreateBlendState(
            &alphaDesc,
            m_alphaBlendState.
            GetAddressOf()
        );

    if (FAILED(hr))
    {
        return false;
    }

    D3D11_BLEND_DESC opaqueDesc{};

    opaqueDesc.RenderTarget[0].
        BlendEnable =
        FALSE;

    opaqueDesc.RenderTarget[0].
        RenderTargetWriteMask =
        D3D11_COLOR_WRITE_ENABLE_ALL;

    hr =
        m_renderer->GetDevice()->
        CreateBlendState(
            &opaqueDesc,
            m_opaqueBlendState.
            GetAddressOf()
        );

    return SUCCEEDED(hr);
}

void SpriteRenderer::SetCamera(
    const Camera& camera)
{
    m_viewMatrix =
        camera.GetViewMatrix();

    m_projectionMatrix =
        camera.GetProjectionMatrix();
}

void SpriteRenderer::SetBlendMode(
    BlendMode mode)
{
    if (m_currentBlendMode ==
        mode)
    {
        return;
    }

    m_currentBlendMode =
        mode;

    float blendFactor[4] =
    {
        0.0f,
        0.0f,
        0.0f,
        0.0f
    };

    ID3D11BlendState* state =
        nullptr;

    switch (mode)
    {
    case BlendMode::Opaque:

        state =
            m_opaqueBlendState.Get();

        break;

    case BlendMode::Alpha:
    default:

        state =
            m_alphaBlendState.Get();

        break;
    }

    m_renderer->GetContext()->
        OMSetBlendState(
            state,
            blendFactor,
            0xffffffff
        );
}