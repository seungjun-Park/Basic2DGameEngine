#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <wrl/client.h>
#include <xaudio2.h>


class AudioClip;


class AudioSystem final
{
public:

    static constexpr std::size_t
        MaxActiveSfxVoices = 64;


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


    //
    // Main-thread maintenance.
    //
    // 완료된 one-shot SourceVoice를 회수한다.
    //
    void Update() noexcept;


    //
    // volume:
    // engine policy상 0.0 ~ 1.0
    //
    bool PlayOneShot(
        const AudioClip& clip,
        float volume = 1.0f
    );


    bool IsInitialized()
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


    ActiveVoice*
        FindFreeVoiceSlot()
        noexcept;


    void DestroySourceVoice(
        ActiveVoice& activeVoice
    ) noexcept;


    void DestroyAllSourceVoices()
        noexcept;


private:

    Microsoft::WRL::ComPtr<
        IXAudio2
    >
        m_xaudio2;


    IXAudio2MasteringVoice*
        m_masterVoice = nullptr;


    std::array<
        ActiveVoice,
        MaxActiveSfxVoices
    >
        m_activeVoices{};


    std::uint32_t
        m_outputChannels = 0;

    std::uint32_t
        m_outputSampleRate = 0;


    bool
        m_initialized = false;
};