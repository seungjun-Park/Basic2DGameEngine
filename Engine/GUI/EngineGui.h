#pragma once

class AudioSystem;

class TileMap;
class TileMapRenderer;
class TileMapCollider;

namespace EngineGui
{
    void DrawAudioSettingsContents(
        AudioSystem& audioSystem);

    void DrawTileMapSettingsContents(
        const TileMap& tileMap,
        TileMapRenderer& renderer,
        const TileMapCollider* collider);
}