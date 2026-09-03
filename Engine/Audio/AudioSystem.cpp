#include "AudioSystem.h"

#include <Windows.h>

#include <cstdio>


#pragma comment(lib, "xaudio2.lib")


namespace
{
    void LogAudioError(
        const char* operation,
        HRESULT hr
    )
    {
        char message[256]{};

        sprintf_s(
            message,
            "[Audio] %s failed. "
            "HRESULT=0x%08lX\n",
            operation,
            static_cast<unsigned long>(
                hr
                )
        );

        OutputDebugStringA(
            message
        );
    }
}


AudioSystem::~AudioSystem()
{
    Shutdown();
}


bool AudioSystem::Initialize()
{
    //
    // Re-initialize도 안전하게 만든다.
    //
    Shutdown();


    HRESULT hr =
        XAudio2Create(
            m_xaudio2.
            ReleaseAndGetAddressOf(),
            0,
            XAUDIO2_DEFAULT_PROCESSOR
        );

    if (FAILED(hr))
    {
        LogAudioError(
            "XAudio2Create",
            hr
        );

        return false;
    }


    //
    // Default output device,
    // default channel count,
    // default sample rate를 사용한다.
    //
    hr =
        m_xaudio2->
        CreateMasteringVoice(
            &m_masterVoice
        );

    if (FAILED(hr))
    {
        LogAudioError(
            "CreateMasteringVoice",
            hr
        );

        Shutdown();

        return false;
    }


    XAUDIO2_VOICE_DETAILS
        voiceDetails{};

    m_masterVoice->
        GetVoiceDetails(
            &voiceDetails
        );


    m_outputChannels =
        voiceDetails.InputChannels;

    m_outputSampleRate =
        voiceDetails.InputSampleRate;


    if (m_outputChannels == 0 ||
        m_outputSampleRate == 0)
    {
        OutputDebugStringA(
            "[Audio] Invalid mastering "
            "voice output configuration.\n"
        );

        Shutdown();

        return false;
    }


    m_initialized =
        true;


    char message[256]{};

    sprintf_s(
        message,
        "[Audio] XAudio2 initialized. "
        "Channels=%u "
        "SampleRate=%u Hz\n",
        m_outputChannels,
        m_outputSampleRate
    );

    OutputDebugStringA(
        message
    );


    return true;
}


void AudioSystem::Shutdown()
noexcept
{
    //
    // 향후 SourceVoice / SubmixVoice가
    // 추가되면 반드시 여기보다 먼저
    // 모두 제거되어야 한다.
    //
    // Source / Submix
    //      ↓
    // MasteringVoice
    //      ↓
    // IXAudio2
    //


    if (m_xaudio2)
    {
        //
        // 새 audio processing을 먼저 멈춘다.
        //
        m_xaudio2->
            StopEngine();
    }


    if (m_masterVoice)
    {
        m_masterVoice->
            DestroyVoice();

        m_masterVoice =
            nullptr;
    }


    m_xaudio2.Reset();


    m_outputChannels =
        0;

    m_outputSampleRate =
        0;

    m_initialized =
        false;
}


bool AudioSystem::IsInitialized()
const noexcept
{
    return
        m_initialized &&
        m_xaudio2 != nullptr &&
        m_masterVoice != nullptr;
}


std::uint32_t
AudioSystem::GetOutputChannels()
const noexcept
{
    return
        m_outputChannels;
}


std::uint32_t
AudioSystem::GetOutputSampleRate()
const noexcept
{
    return
        m_outputSampleRate;
}