#include "ResourceManager.h"

#include "Engine/Graphics/Texture.h"
#include "Engine/Renderer/DX11Renderer.h"

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

bool ResourceManager::Initialize(
    DX11Renderer& renderer)
{
    m_renderer = &renderer;

    return true;
}

Texture* ResourceManager::LoadTexture(
    const std::wstring& path)
{
    auto it =
        m_textures.find(path);

    if (it != m_textures.end())
    {
        OutputDebugStringA(
            "[Resource] Texture cache hit\n"
        );

        return it->second.get();
    }

    auto texture =
        std::make_unique<Texture>();

    if (!texture->Load(
        m_renderer->GetDevice(),
        path))
    {
        return nullptr;
    }

    Texture* result =
        texture.get();

    m_textures.emplace(
        path,
        std::move(texture)
    );

    OutputDebugStringA(
        "[Resource] Texture loaded\n"
    );

    return result;
}

void ResourceManager::Clear()
{
    m_textures.clear();
}