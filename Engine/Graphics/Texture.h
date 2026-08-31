#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <string>

struct UVBuffer
{
    DirectX::XMFLOAT4 uvRect;
};

class Texture
{
public:
    Texture() = default;
    ~Texture() = default;

    bool Load(
        ID3D11Device* device,
        const std::wstring& path
    );

    ID3D11ShaderResourceView* GetSRV() const
    {
        return m_shaderResourceView.Get();
    }

    int GetWidth() const
    {
        return m_width;
    }

    int GetHeight() const
    {
        return m_height;
    }

    bool CreateUVBuffer();

private:
    Microsoft::WRL::ComPtr<
        ID3D11ShaderResourceView
    > m_shaderResourceView;

    Microsoft::WRL::ComPtr<
        ID3D11Buffer
    > m_uvBuffer;

    int m_width = 0;
    int m_height = 0;
};