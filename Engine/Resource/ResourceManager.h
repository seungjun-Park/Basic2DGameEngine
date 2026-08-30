#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Texture;
class DX11Renderer;

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    bool Initialize(
        DX11Renderer& renderer
    );

    Texture* LoadTexture(
        const std::wstring& path
    );

    void Clear();

private:
    DX11Renderer* m_renderer = nullptr;

    std::unordered_map<
        std::wstring,
        std::unique_ptr<Texture>
    > m_textures;
};
