#include "EngineGui.h"

#include "Engine/Audio/AudioSystem.h"
#include "Engine/Tile/TileMap.h"
#include "Engine/Tile/TileMapRenderer.h"
#include "Engine/Tile/TileMapCollider.h"

#include <imgui.h>

#include <cstddef>

namespace EngineGui
{
    void DrawAudioSettings(
        AudioSystem& audioSystem)
    {
        ImGui::Begin("Audio Settings");

        if (!audioSystem.IsInitialized())
        {
            ImGui::TextDisabled(
                "AudioSystem is not initialized.");

            ImGui::End();

            return;
        }

        float masterVolume =
            audioSystem.GetMasterVolume();

        float sfxVolume =
            audioSystem.GetSfxVolume();

        float musicVolume =
            audioSystem.GetMusicVolume();

        bool volumeUpdateFailed = false;

        if (ImGui::SliderFloat(
            "Master",
            &masterVolume,
            0.0f,
            1.0f,
            "%.2f",
            ImGuiSliderFlags_AlwaysClamp))
        {
            if (!audioSystem.SetMasterVolume(
                masterVolume))
            {
                volumeUpdateFailed = true;
            }
        }

        if (ImGui::SliderFloat(
            "SFX",
            &sfxVolume,
            0.0f,
            1.0f,
            "%.2f",
            ImGuiSliderFlags_AlwaysClamp))
        {
            if (!audioSystem.SetSfxVolume(
                sfxVolume))
            {
                volumeUpdateFailed = true;
            }
        }

        if (ImGui::SliderFloat(
            "Music",
            &musicVolume,
            0.0f,
            1.0f,
            "%.2f",
            ImGuiSliderFlags_AlwaysClamp))
        {
            if (!audioSystem.SetMusicVolume(
                musicVolume))
            {
                volumeUpdateFailed = true;
            }
        }

        if (volumeUpdateFailed)
        {
            ImGui::Separator();

            ImGui::TextDisabled(
                "Audio volume update failed.");
        }

        ImGui::Separator();

        ImGui::Text(
            "Output: %u channels / %u Hz",
            audioSystem.GetOutputChannels(),
            audioSystem.GetOutputSampleRate());

        ImGui::Text(
            "Active SFX Voices: %zu / %zu",
            audioSystem.GetActiveVoiceCount(),
            AudioSystem::MaxActiveSfxVoices);

        ImGui::Text(
            "Persistent Voices: %zu / %zu",
            audioSystem.GetPersistentVoiceCount(),
            AudioSystem::MaxPersistentVoices);

        ImGui::End();
    }

    void DrawTileMapSettings(
        const TileMap& tileMap,
        TileMapRenderer& renderer,
        const TileMapCollider* collider)
    {
        ImGui::Begin("TileMap Settings");

        ImGui::Text(
            "Map Size: %d x %d",
            tileMap.GetWidth(),
            tileMap.GetHeight());

        ImGui::Text(
            "Tile Size: %d x %d",
            tileMap.GetTileWidth(),
            tileMap.GetTileHeight());

        ImGui::Text(
            "Layers: %zu",
            tileMap.GetLayerCount());

        ImGui::TextDisabled(
            "Map and tile dimensions are read-only at runtime.");

        ImGui::SeparatorText(
            "Layers");

        for (std::size_t i = 0;
            i < tileMap.GetLayerCount();
            ++i)
        {
            const TileLayer* layer =
                tileMap.GetLayer(i);

            if (!layer)
            {
                continue;
            }

            ImGui::PushID(
                static_cast<int>(i));

            const char* layerName =
                layer->name.empty()
                ? "<Unnamed>"
                : layer->name.c_str();

            if (layer->type ==
                TileLayerType::Render)
            {
                if (renderer.
                    IsRenderLayerCached(i))
                {
                    bool visible =
                        renderer.
                        IsRenderLayerVisible(i);

                    if (ImGui::Checkbox(
                        "##Visible",
                        &visible))
                    {
                        renderer.
                            SetRenderLayerVisible(
                                i,
                                visible);
                    }

                    ImGui::SameLine();

                    ImGui::Text(
                        "%s [Render]",
                        layerName);
                }
                else
                {
                    ImGui::TextDisabled(
                        "%s [Render - unavailable]",
                        layerName);
                }
            }
            else
            {
                ImGui::BulletText(
                    "%s [Collision]",
                    layerName);

                ImGui::SameLine();

                ImGui::TextDisabled(
                    "(physics always active)");
            }

            ImGui::PopID();
        }

        ImGui::SeparatorText(
            "Renderer");

        const TileMapRenderStats& stats =
            renderer.GetStats();

        ImGui::Text(
            "Active Render Layers: %zu",
            stats.renderLayerCount);

        ImGui::Text(
            "Active Render Items: %zu",
            renderer.GetRenderItemCount());

        ImGui::Text(
            "Visible Tiles: %zu",
            stats.visibleRenderItems);

        ImGui::Text(
            "Culled Tiles: %zu",
            stats.culledRenderItems);

        ImGui::SeparatorText(
            "Collision");

        if (collider)
        {
            ImGui::Text(
                "Collision Layers: %zu",
                collider->
                GetCollisionLayerCount());

            ImGui::Text(
                "Solid Tiles: %zu",
                collider->
                GetSolidTileCount());

            ImGui::Text(
                "Collision Shapes: %zu",
                collider->
                GetShapeCount());

            ImGui::Text(
                "Merged Tile Area: %zu",
                collider->
                GetMergedTileArea());
        }
        else
        {
            ImGui::TextDisabled(
                "TileMap collider unavailable.");
        }

        ImGui::End();
    }
}