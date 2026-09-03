#include "AudioClip.h"

#include <utility>


AudioClip::AudioClip(
    const WAVEFORMATEX& format,
    std::vector<std::uint8_t> audioData)
    :
    m_format(format),
    m_audioData(
        std::move(audioData)
    )
{
    if (m_format.nAvgBytesPerSec > 0)
    {
        m_durationSeconds =
            static_cast<float>(
                m_audioData.size()
                ) /
            static_cast<float>(
                m_format.nAvgBytesPerSec
                );
    }
}


const WAVEFORMATEX&
AudioClip::GetFormat() const noexcept
{
    return m_format;
}


const std::uint8_t*
AudioClip::GetAudioData() const noexcept
{
    if (m_audioData.empty())
    {
        return nullptr;
    }

    return
        m_audioData.data();
}


std::size_t
AudioClip::GetAudioByteCount() const noexcept
{
    return
        m_audioData.size();
}


float
AudioClip::GetDurationSeconds() const noexcept
{
    return
        m_durationSeconds;
}


bool AudioClip::IsValid()
const noexcept
{
    return
        m_format.wFormatTag ==
        WAVE_FORMAT_PCM &&
        m_format.nChannels > 0 &&
        m_format.nSamplesPerSec > 0 &&
        m_format.nBlockAlign > 0 &&
        m_format.nAvgBytesPerSec > 0 &&
        m_format.wBitsPerSample > 0 &&
        !m_audioData.empty();
}