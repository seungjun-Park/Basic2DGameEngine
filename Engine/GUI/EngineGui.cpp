#include "EngineGui.h"

#include "Engine/Audio/AudioSystem.h"

#include <imgui.h>

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
}