#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <DirectXMath.h>
#include <cstddef>

#include "RenderTypes.h"

class DX11Renderer;
class Transform;
class Texture;
class Camera;
struct Sprite;
struct SpriteRenderCommand;

class SpriteRenderer
{
public:
    SpriteRenderer() = default;
    ~SpriteRenderer() = default;

    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;

    bool Initialize(
        DX11Renderer& renderer,
        int screenWidth,
        int screenHeight
    );

    void Begin();

    void Draw(
        const Sprite& sprite,
        const Transform& transform
    );

    void DrawBatch(
        const SpriteRenderCommand* commands,
        std::size_t commandCount
    );

    void End();

    void SetCamera(
        const Camera& camera
    );

    void SetBlendMode(
        BlendMode mode
    );

    int GetDrawCallCount() const
    {
        return m_drawCallCount;
    }

private:
    bool CreateShaders();
    bool CreateInputLayout();
    bool CreateBuffers();
    bool CreateSampler();
    bool CreateBlendStates();

    bool CompileShader(
        const wchar_t* filename,
        const char* entryPoint,
        const char* target,
        Microsoft::WRL::ComPtr<ID3DBlob>& blob
    );

private:
    DX11Renderer* m_renderer = nullptr;

    Microsoft::WRL::ComPtr<ID3D11VertexShader>
        m_vertexShader;

    Microsoft::WRL::ComPtr<ID3D11PixelShader>
        m_pixelShader;

    Microsoft::WRL::ComPtr<ID3D11InputLayout>
        m_inputLayout;

    Microsoft::WRL::ComPtr<ID3D11Buffer>
        m_vertexBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer>
        m_indexBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer>
        m_constantBuffer;

    Microsoft::WRL::ComPtr<ID3D11SamplerState>
        m_samplerState;

    Microsoft::WRL::ComPtr<ID3D11BlendState> 
        m_blendState;

    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_projectionMatrix;

    int m_screenWidth = 0;
    int m_screenHeight = 0;

    int m_drawCallCount = 0;

private:
    Microsoft::WRL::ComPtr<
        ID3D11BlendState
    > m_alphaBlendState;

    Microsoft::WRL::ComPtr<
        ID3D11BlendState
    > m_opaqueBlendState;

    BlendMode m_currentBlendMode =
        BlendMode::Alpha;
};
