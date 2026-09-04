#include "ResourceManager.h"

#include "Engine/Animation/AnimationClip.h"
#include "Engine/Animation/AnimationClipLoader.h"
#include "Engine/Audio/AudioClip.h"
#include "Engine/Audio/AudioClipLoader.h"
#include "Engine/Debug/DebugLog.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Renderer/DX11Renderer.h"
#include "Engine/Tile/TileMap.h"
#include "Engine/Tile/TileMapLoader.h"
#include "Engine/Tile/Tileset.h"
#include "Engine/Tile/TilesetLoader.h"

#include <utility>

ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager()
{
    ReleaseAllResources();
}

bool ResourceManager::Initialize(
    DX11Renderer& renderer)
{
    m_renderer = &renderer;

    return true;
}

Texture* ResourceManager::LoadTexture(
    const std::wstring& path)
{
    if (!m_renderer)
    {
        ENGINE_DEBUG_LOG(
            "[Resource] LoadTexture called "
            "before initialization.\n"
        );

        return nullptr;
    }

    auto it =
        m_textures.find(path);

    if (it != m_textures.end())
    {
        ENGINE_DEBUG_LOG(
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

    m_texturePathsByPointer.emplace(
        result,
        path
    );

    ENGINE_DEBUG_LOG(
        "[Resource] Texture loaded\n"
    );

    return result;
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
        ENGINE_DEBUG_LOG(
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

    if (!IsTextureManaged(
        animationClip->GetTexture()))
    {
        ENGINE_DEBUG_LOG(
            "[Resource] AnimationClip "
            "references an unmanaged texture.\n"
        );

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

    ENGINE_DEBUG_LOG(
        "[Resource] Animation clip loaded\n"
    );

    return result;
}

AudioClip* ResourceManager::LoadAudioClip(
    const std::wstring& path)
{
    const auto cached =
        m_audioClips.find(
            path
        );

    if (cached !=
        m_audioClips.end())
    {
        return
            cached->second.get();
    }


    auto audioClip =
        AudioClipLoader::Load(
            path
        );

    if (!audioClip)
    {
        ENGINE_DEBUG_LOG(
            "[Resource] Failed to "
            "load AudioClip.\n"
        );

        return nullptr;
    }


    AudioClip* result =
        audioClip.get();


    m_audioClips.emplace(
        path,
        std::move(audioClip)
    );


    ENGINE_DEBUG_LOG(
        "[Resource] AudioClip loaded.\n"
    );


    return result;
}

bool ResourceManager::TryGetTexturePath(
    const Texture* texture,
    std::wstring& outPath) const
{
    outPath.clear();

    if (!texture)
    {
        return false;
    }

    const auto it =
        m_texturePathsByPointer.find(
            texture
        );

    if (it ==
        m_texturePathsByPointer.end())
    {
        return false;
    }

    outPath =
        it->second;

    return true;
}

bool ResourceManager::IsTextureManaged(
    const Texture* texture) const noexcept
{
    if (!texture)
    {
        return false;
    }

    return
        m_texturePathsByPointer.find(
            texture
        ) !=
        m_texturePathsByPointer.end();
}

void ResourceManager::ReleaseAllResources() noexcept
{
    m_texturePathsByPointer.clear();

    m_tileMaps.clear();

    m_animationClips.clear();

    m_audioClips.clear();

    m_tilesets.clear();

    m_textures.clear();

    m_renderer =
        nullptr;
}