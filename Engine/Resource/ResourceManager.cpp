#include "ResourceManager.h"

#include "Engine/Graphics/Texture.h"
#include "Engine/Renderer/DX11Renderer.h"
#include "Engine/Tile/TilesetLoader.h"
#include "Engine/Tile/TileMapLoader.h"
#include "Engine/Animation/AnimationClip.h"
#include "Engine/Animation/AnimationClipLoader.h"

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
    m_tileMaps.clear();

    m_animationClips.clear();

    m_tilesets.clear();

    m_textures.clear();
}

Tileset*
ResourceManager::LoadTileset(
    const std::wstring& path)
{
    auto it =
        m_tilesets.find(
            path
        );

    if (it !=
        m_tilesets.end())
    {
        return
            it->second.get();
    }

    auto tileset =
        TilesetLoader::Load(
            path,
            *this
        );

    if (!tileset)
    {
        return nullptr;
    }

    Tileset* result =
        tileset.get();

    m_tilesets.emplace(
        path,
        std::move(tileset)
    );

    return result;
}

TileMap*
ResourceManager::LoadTileMap(
    const std::wstring& path)
{
    auto it =
        m_tileMaps.find(
            path
        );

    if (it !=
        m_tileMaps.end())
    {
        return
            it->second.get();
    }

    auto tileMap =
        TileMapLoader::Load(
            path,
            *this
        );

    if (!tileMap)
    {
        return nullptr;
    }

    TileMap* result =
        tileMap.get();

    m_tileMaps.emplace(
        path,
        std::move(tileMap)
    );

    return result;
}

AnimationClip*
ResourceManager::LoadAnimationClip(
    const std::wstring& path)
{
    auto it =
        m_animationClips.find(
            path
        );

    if (it !=
        m_animationClips.end())
    {
        OutputDebugStringA(
            "[Resource] Animation clip cache hit\n"
        );

        return
            it->second.get();
    }

    auto animationClip =
        AnimationClipLoader::Load(
            path,
            *this
        );

    if (!animationClip)
    {
        return nullptr;
    }

    AnimationClip* result =
        animationClip.get();

    m_animationClips.emplace(
        path,
        std::move(
            animationClip
        )
    );

    OutputDebugStringA(
        "[Resource] Animation clip loaded\n"
    );

    return result;
}