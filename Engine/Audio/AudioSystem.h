#pragma once

#include <cstdint>

#include <wrl/client.h>
#include <xaudio2.h>


class AudioSystem final
{
public:

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


    bool IsInitialized()
        const noexcept;


    std::uint32_t
        GetOutputChannels()
        const noexcept;

    std::uint32_t
        GetOutputSampleRate()
        const noexcept;


private:

    //
    // IXAudio2는 AddRef / Release를 지원하므로
    // COM smart pointer로 소유한다.
    //
    Microsoft::WRL::ComPtr<
        IXAudio2
    >
        m_xaudio2;


    //
    // XAudio2 Voice interface에는
    // Release()가 없다.
    //
    // 반드시 DestroyVoice()로 파괴한다.
    //
    IXAudio2MasteringVoice*
        m_masterVoice = nullptr;


    std::uint32_t
        m_outputChannels = 0;

    std::uint32_t
        m_outputSampleRate = 0;


    bool
        m_initialized = false;
};