#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Texture;
class Tileset;
class TileMap;
class DX11Renderer;
class AnimationClip;

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

    bool TryGetTexturePath(
        const Texture* texture,
        std::wstring& outPath
    ) const;

    bool IsTextureManaged(
        const Texture* texture
    ) const noexcept;

    Tileset* LoadTileset(
        const std::wstring& path
    );

    TileMap* LoadTileMap(
        const std::wstring& path
    );

    AnimationClip* LoadAnimationClip(
        const std::wstring& path
    );

private:
    void ReleaseAllResources() noexcept;

private:
    DX11Renderer* m_renderer = nullptr;

    std::unordered_map<
        std::wstring,
        std::unique_ptr<Texture>
    > m_textures;

    std::unordered_map<
        const Texture*,
        std::wstring
    > m_texturePathsByPointer;

    std::unordered_map<
        std::wstring,
        std::unique_ptr<Tileset>
    > m_tilesets;

    std::unordered_map<
        std::wstring,
        std::unique_ptr<TileMap>
    > m_tileMaps;

    std::unordered_map<
        std::wstring,
        std::unique_ptr<AnimationClip>
    > m_animationClips;
};
