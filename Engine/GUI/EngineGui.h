#pragma once

class AudioSystem;

class TileMap;
class TileMapRenderer;
class TileMapCollider;

namespace EngineGui
{
    void DrawAudioSettings(
        AudioSystem& audioSystem);

    void DrawTileMapSettings(
        const TileMap& tileMap,
        TileMapRenderer& renderer,
        const TileMapCollider* collider);
}