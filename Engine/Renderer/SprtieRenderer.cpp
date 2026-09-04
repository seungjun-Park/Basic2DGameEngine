#include "SpriteRenderer.h"
#include "Camera.h"

#include "DX11Renderer.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Components/Sprite.h"
#include "Engine/Debug/DebugLog.h"
#include "Engine/Components/Transform.h"
#include "RenderCommand.h"

#include <d3dcompiler.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

#pragma comment(lib, "d3dcompiler.lib")

namespace
{
    constexpr std::size_t MaxBatchSprites =
        2048;

    constexpr std::size_t VerticesPerSprite =
        4;

    constexpr std::size_t IndicesPerSprite =
        6;

    struct SpriteVertex
    {
        XMFLOAT3 position;
        XMFLOAT2 uv;
    };

    struct SpriteFrameConstantBuffer
    {
        XMMATRIX view;
        XMMATRIX projection;
    };

    static_assert(
        sizeof(SpriteFrameConstantBuffer) % 16 == 0,
        "Constant buffer size must be a multiple of 16 bytes."
        );

    XMMATRIX BuildSpriteWorldMatrix(
        const Transform& transform)
    {
        //
        // IMPORTANT:
        //
        // Phase 7에서 transform semantic을 변경하지 않는다.
        // 기존 SpriteRenderer::Draw()가 사용하던
        // World matrix 계산식을 그대로 유지한다.
        //
        return
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
    }

    void WriteSpriteVertices(
        SpriteVertex* destination,
        const Sprite& sprite,
        const Transform& transform)
    {
        const XMMATRIX world =
            BuildSpriteWorldMatrix(
                transform
            );

        const XMVECTOR topLeft =
            XMVector3TransformCoord(
                XMVectorSet(
                    0.0f,
                    0.0f,
                    0.0f,
                    1.0f
                ),
                world
            );

        const XMVECTOR topRight =
            XMVector3TransformCoord(
                XMVectorSet(
                    1.0f,
                    0.0f,
                    0.0f,
                    1.0f
                ),
                world
            );

        const XMVECTOR bottomLeft =
            XMVector3TransformCoord(
                XMVectorSet(
                    0.0f,
                    1.0f,
                    0.0f,
                    1.0f
                ),
                world
            );

        const XMVECTOR bottomRight =
            XMVector3TransformCoord(
                XMVectorSet(
                    1.0f,
                    1.0f,
                    0.0f,
                    1.0f
                ),
                world
            );

        XMStoreFloat3(
            &destination[0].position,
            topLeft
        );

        XMStoreFloat3(
            &destination[1].position,
            topRight
        );

        XMStoreFloat3(
            &destination[2].position,
            bottomLeft
        );

        XMStoreFloat3(
            &destination[3].position,
            bottomRight
        );

        destination[0].uv =
            XMFLOAT2(
                sprite.uv.u0,
                sprite.uv.v0
            );

        destination[1].uv =
            XMFLOAT2(
                sprite.uv.u1,
                sprite.uv.v0
            );

        destination[2].uv =
            XMFLOAT2(
                sprite.uv.u0,
                sprite.uv.v1
            );

        destination[3].uv =
            XMFLOAT2(
                sprite.uv.u1,
                sprite.uv.v1
            );
    }
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

    SpriteFrameConstantBuffer
        frameBuffer{};

    frameBuffer.view =
        XMMatrixTranspose(
            m_viewMatrix
        );

    frameBuffer.projection =
        XMMatrixTranspose(
            m_projectionMatrix
        );

    D3D11_MAPPED_SUBRESOURCE
        mapped{};

    HRESULT hr =
        context->Map(
            m_constantBuffer.Get(),
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &mapped
        );

    if (FAILED(hr))
    {
        return;
    }

    memcpy(
        mapped.pData,
        &frameBuffer,
        sizeof(frameBuffer)
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

void SpriteRenderer::Draw(
    const Sprite& sprite,
    const Transform& transform)
{
    if (!sprite.visible)
    {
        return;
    }

    if (!sprite.texture)
    {
        return;
    }

    SpriteRenderCommand command;

    command.sprite =
        &sprite;

    command.transform =
        &transform;

    command.texture =
        sprite.texture;

    command.layer =
        sprite.layer;

    command.blendMode =
        sprite.blendMode;

    SetBlendMode(
        command.blendMode
    );

    DrawBatch(
        &command,
        1
    );
}

void SpriteRenderer::DrawBatch(
    const SpriteRenderCommand* commands,
    std::size_t commandCount)
{
    if (!commands ||
        commandCount == 0)
    {
        return;
    }

    auto* context =
        m_renderer->GetContext();

    std::size_t commandOffset = 0;

    while (commandOffset <
        commandCount)
    {
        const std::size_t
            remainingCount =
            commandCount -
            commandOffset;

        const std::size_t batchCount =
            std::min(
                MaxBatchSprites,
                remainingCount
            );

#ifdef _DEBUG

        const SpriteRenderCommand&
            firstCommand =
            commands[
                commandOffset
            ];

        assert(
            IsSpriteRenderCommandValid(
                firstCommand
            )
        );

        assert(
            firstCommand.sprite
        );

        assert(
            firstCommand.transform
        );

        assert(
            firstCommand.sprite->texture
        );

        for (std::size_t index = 0;
            index < batchCount;
            ++index)
        {
            const SpriteRenderCommand&
                command =
                commands[
                    commandOffset +
                        index
                ];

            assert(
                command.sprite
            );

            assert(
                command.transform
            );

            assert(
                command.sprite->texture
            );

            assert(
                CanBatchSpriteRenderCommands(
                    firstCommand,
                    command
                )
            );

            assert(
                command.sprite->texture ==
                command.texture
            );
        }

#endif

        D3D11_MAPPED_SUBRESOURCE
            mapped{};

        HRESULT hr =
            context->Map(
                m_vertexBuffer.Get(),
                0,
                D3D11_MAP_WRITE_DISCARD,
                0,
                &mapped
            );

        if (FAILED(hr))
        {
            return;
        }

        SpriteVertex* vertices =
            static_cast<SpriteVertex*>(
                mapped.pData
                );

        for (std::size_t index = 0;
            index < batchCount;
            ++index)
        {
            const SpriteRenderCommand&
                command =
                commands[
                    commandOffset +
                        index
                ];

            WriteSpriteVertices(
                vertices +
                index *
                VerticesPerSprite,
                *command.sprite,
                *command.transform
            );
        }

        context->Unmap(
            m_vertexBuffer.Get(),
            0
        );

        Texture* texture =
            commands[
                commandOffset
            ].texture;

        ID3D11ShaderResourceView* srv =
            texture->GetSRV();

        context->PSSetShaderResources(
            0,
            1,
            &srv
        );

        context->DrawIndexed(
            static_cast<UINT>(
                batchCount *
                IndicesPerSprite
                ),
            0,
            0
        );

        ++m_drawCallCount;

        commandOffset +=
            batchCount;
    }
}

void SpriteRenderer::End()
{
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
    ID3D11Device* device =
        m_renderer->GetDevice();

    D3D11_BUFFER_DESC
        vertexBufferDesc{};

    vertexBufferDesc.Usage =
        D3D11_USAGE_DYNAMIC;

    vertexBufferDesc.ByteWidth =
        static_cast<UINT>(
            sizeof(SpriteVertex) *
            VerticesPerSprite *
            MaxBatchSprites
            );

    vertexBufferDesc.BindFlags =
        D3D11_BIND_VERTEX_BUFFER;

    vertexBufferDesc.CPUAccessFlags =
        D3D11_CPU_ACCESS_WRITE;

    HRESULT hr =
        device->CreateBuffer(
            &vertexBufferDesc,
            nullptr,
            m_vertexBuffer.GetAddressOf()
        );

    if (FAILED(hr))
    {
        return false;
    }

    std::vector<std::uint32_t> indices(
        MaxBatchSprites *
        IndicesPerSprite
    );

    for (std::size_t spriteIndex = 0;
        spriteIndex < MaxBatchSprites;
        ++spriteIndex)
    {
        const std::uint32_t baseVertex =
            static_cast<std::uint32_t>(
                spriteIndex *
                VerticesPerSprite
                );

        const std::size_t baseIndex =
            spriteIndex *
            IndicesPerSprite;

        indices[baseIndex + 0] =
            baseVertex + 0;

        indices[baseIndex + 1] =
            baseVertex + 1;

        indices[baseIndex + 2] =
            baseVertex + 2;

        indices[baseIndex + 3] =
            baseVertex + 2;

        indices[baseIndex + 4] =
            baseVertex + 1;

        indices[baseIndex + 5] =
            baseVertex + 3;
    }

    D3D11_BUFFER_DESC
        indexBufferDesc{};

    indexBufferDesc.Usage =
        D3D11_USAGE_DEFAULT;

    indexBufferDesc.ByteWidth =
        static_cast<UINT>(
            sizeof(std::uint32_t) *
            indices.size()
            );

    indexBufferDesc.BindFlags =
        D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA
        indexData{};

    indexData.pSysMem =
        indices.data();

    hr =
        device->CreateBuffer(
            &indexBufferDesc,
            &indexData,
            m_indexBuffer.GetAddressOf()
        );

    if (FAILED(hr))
    {
        return false;
    }

    D3D11_BUFFER_DESC
        constantBufferDesc{};

    constantBufferDesc.Usage =
        D3D11_USAGE_DYNAMIC;

    constantBufferDesc.ByteWidth =
        sizeof(
            SpriteFrameConstantBuffer
            );

    constantBufferDesc.BindFlags =
        D3D11_BIND_CONSTANT_BUFFER;

    constantBufferDesc.CPUAccessFlags =
        D3D11_CPU_ACCESS_WRITE;

    hr =
        device->CreateBuffer(
            &constantBufferDesc,
            nullptr,
            m_constantBuffer.GetAddressOf()
        );

    if (FAILED(hr))
    {
        return false;
    }

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
            ENGINE_DEBUG_LOG(
                static_cast<const char*>(
                    errorBlob->GetBufferPointer()
                    )
            );
        }

        return false;
    }

    return true;
}
