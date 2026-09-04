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

    AudioClip(
        const AudioClip&
    ) = delete;

    AudioClip& operator=(
        const AudioClip&
        ) = delete;

    AudioClip(
        AudioClip&&
    ) = delete;

    AudioClip& operator=(
        AudioClip&&
        ) = delete;


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

    std::vector<std::uint8_t>
        m_audioData;

    float
        m_durationSeconds = 0.0f;
};