#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <Windows.h>
#include <mmreg.h>


class AudioClip final
{
public:

    AudioClip(
        const WAVEFORMATEX& format,
        std::vector<std::uint8_t> audioData
    );


    const WAVEFORMATEX&
        GetFormat() const noexcept;


    const std::uint8_t*
        GetAudioData() const noexcept;


    std::size_t
        GetAudioByteCount() const noexcept;


    float
        GetDurationSeconds() const noexcept;


    bool IsValid()
        const noexcept;


private:

    WAVEFORMATEX
        m_format{};

    //
    // XAudio2 playback 시 pAudioData가
    // 이 memory를 직접 참조하게 된다.
    //
    // Load 후에는 resize/modify하지 않는다.
    //
    std::vector<std::uint8_t>
        m_audioData;

    float
        m_durationSeconds = 0.0f;
};