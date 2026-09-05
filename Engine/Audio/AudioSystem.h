#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <wrl/client.h>
#include <xaudio2.h>

#include "AudioPlaybackHandle.h"


class AudioClip;


enum class AudioCategory :
    std::uint8_t
{
    Sfx = 0,
    Music
};


class AudioSystem final
{
public:
    static constexpr std::size_t
        MaxActiveSfxVoices = 64;
    static constexpr std::size_t
        MaxPersistentVoices = 8;


    AudioSystem() = default;
    ~AudioSystem();
    AudioSystem(
        const AudioSystem&
    ) = delete;
    AudioSystem&
        operator=(
            const AudioSystem&
            ) = delete;
    AudioSystem(
        AudioSystem&&
    ) = delete;
    AudioSystem&
        operator=(
            AudioSystem&&
            ) = delete;


    bool Initialize();
    void Shutdown() noexcept;


    void Update() noexcept;

    //
    // AudioClip data is referenced directly by XAudio2.
    //
    // The clip must remain alive and unmodified until
    // the submitted playback has completed or been stopped.
    //
    // Engine resource policy:
    // use ResourceManager-managed AudioClip instances.
    //

    bool PlayOneShot(
        const AudioClip& clip,
        float volume = 1.0f
    );


    [[nodiscard]]
    AudioPlaybackHandle PlayLoop(
        const AudioClip& clip,
        float volume = 1.0f
    );
    [[nodiscard]]
    AudioPlaybackHandle PlayLoop(
        const AudioClip& clip,
        AudioCategory category,
        float volume
    );
    [[nodiscard]]
    AudioPlaybackHandle PlayMusic(
        const AudioClip& clip,
        float volume = 1.0f
    );


    bool Pause(
        AudioPlaybackHandle handle
    );
    bool Resume(
        AudioPlaybackHandle handle
    );
    bool Stop(
        AudioPlaybackHandle handle
    );


    bool SetMasterVolume(
        float volume
    );
    bool SetSfxVolume(
        float volume
    );
    bool SetMusicVolume(
        float volume
    );
    bool SetSuspended(
        bool suspended
    ) noexcept;


    [[nodiscard]]
    bool IsSuspended()
        const noexcept;
    bool IsPlaybackValid(
        AudioPlaybackHandle handle
    ) const noexcept;
    bool IsPaused(
        AudioPlaybackHandle handle
    ) const noexcept;
    bool IsInitialized()
        const noexcept;

    std::size_t
        GetPersistentVoiceCount()
        const noexcept;
    float GetMasterVolume()
        const noexcept;
    float GetSfxVolume()
        const noexcept;
    float GetMusicVolume()
        const noexcept;

    std::uint32_t
        GetOutputChannels()
        const noexcept;
    std::uint32_t
        GetOutputSampleRate()
        const noexcept;
    std::size_t
        GetActiveVoiceCount()
        const noexcept;

private:
    struct ActiveVoice
    {
        IXAudio2SourceVoice*
            voice = nullptr;
    };

    struct PersistentVoice
    {
        AudioPlaybackHandle
            handle{};

        IXAudio2SourceVoice*
            voice = nullptr;

        bool paused =
            false;
    };

    static AudioPlaybackHandle
        AllocatePlaybackHandle();

    HRESULT CreateRoutedSourceVoice(
        IXAudio2SourceVoice** outVoice,
        const WAVEFORMATEX& format,
        AudioCategory category
    ) noexcept;


    void DestroySourceVoice(
        ActiveVoice& activeVoice
    ) noexcept;

    void DestroyAllSourceVoices()
        noexcept;

    void DestroyPersistentVoice(
        PersistentVoice& persistentVoice
    ) noexcept;

    void DestroyAllPersistentVoices()
        noexcept;


    ActiveVoice*
        FindFreeVoiceSlot()
        noexcept;

    PersistentVoice*
        FindFreePersistentVoice()
        noexcept;

    PersistentVoice*
        FindPersistentVoice(
            AudioPlaybackHandle handle
        ) noexcept;

    const PersistentVoice*
        FindPersistentVoice(
            AudioPlaybackHandle handle
        ) const noexcept;


    IXAudio2Voice*
        GetCategoryOutputVoice(
            AudioCategory category
        ) const noexcept;


private:

    Microsoft::WRL::ComPtr<IXAudio2>
        m_xaudio2;


    IXAudio2MasteringVoice*
        m_masterVoice = nullptr;
    IXAudio2SubmixVoice*
        m_sfxSubmixVoice = nullptr;
    IXAudio2SubmixVoice*
        m_musicSubmixVoice = nullptr;

    std::array<
        ActiveVoice,
        MaxActiveSfxVoices
    >
        m_activeVoices{};
    std::array<
        PersistentVoice,
        MaxPersistentVoices
    >
        m_persistentVoices{};


    float m_masterVolume =
        1.0f;
    float m_sfxVolume =
        1.0f;
    float m_musicVolume =
        1.0f;


    std::uint32_t
        m_outputChannels = 0;
    std::uint32_t
        m_outputSampleRate = 0;

    bool
        m_initialized = false;

    bool m_suspended = false;
};