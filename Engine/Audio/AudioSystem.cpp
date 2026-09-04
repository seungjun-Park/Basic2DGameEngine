#include "AudioSystem.h"

#include "AudioClip.h"
#include "Engine/Debug/DebugLog.h"

#include <Windows.h>

#include <atomic>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>


#pragma comment(lib, "xaudio2.lib")



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
        AUDIO_DEBUG_LOG(
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
        AUDIO_DEBUG_LOG(
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

    hr =
        m_xaudio2->
        CreateSubmixVoice(
            &m_sfxSubmixVoice,
            m_outputChannels,
            m_outputSampleRate
        );

    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "CreateSubmixVoice(SFX)",
            hr
        );

        Shutdown();

        return false;
    }


    hr =
        m_xaudio2->
        CreateSubmixVoice(
            &m_musicSubmixVoice,
            m_outputChannels,
            m_outputSampleRate
        );

    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "CreateSubmixVoice(Music)",
            hr
        );

        Shutdown();

        return false;
    }

    hr =
        m_musicSubmixVoice->
        SetVolume(
            m_musicVolume
        );

    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SetVolume(Music)",
            hr
        );

        Shutdown();

        return false;
    }

    if (m_outputChannels == 0 ||
        m_outputSampleRate == 0)
    {
        ENGINE_DEBUG_LOG(
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

    ENGINE_DEBUG_LOG(
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

    DestroyAllSourceVoices();

    DestroyAllPersistentVoices();

    if (m_sfxSubmixVoice)
    {
        m_sfxSubmixVoice->
            DestroyVoice();

        m_sfxSubmixVoice =
            nullptr;
    }


    if (m_musicSubmixVoice)
    {
        m_musicSubmixVoice->
            DestroyVoice();

        m_musicSubmixVoice =
            nullptr;
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
        m_masterVoice != nullptr &&
        m_sfxSubmixVoice != nullptr &&
        m_musicSubmixVoice != nullptr;
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

void AudioSystem::Update()
noexcept
{
    if (!IsInitialized())
    {
        return;
    }


    for (ActiveVoice& activeVoice :
        m_activeVoices)
    {
        if (!activeVoice.voice)
        {
            continue;
        }


        XAUDIO2_VOICE_STATE
            state{};


        activeVoice.voice->
            GetState(
                &state,
                XAUDIO2_VOICE_NOSAMPLESPLAYED
            );


        //
        // Queue가 비었다는 것은
        // 제출한 one-shot buffer가 모두
        // 소비되었다는 의미.
        //
        if (state.BuffersQueued != 0)
        {
            continue;
        }


        DestroySourceVoice(
            activeVoice
        );
    }
}

AudioSystem::ActiveVoice*
AudioSystem::FindFreeVoiceSlot()
noexcept
{
    for (ActiveVoice& activeVoice :
        m_activeVoices)
    {
        if (!activeVoice.voice)
        {
            return
                &activeVoice;
        }
    }

    return nullptr;
}

void AudioSystem::DestroySourceVoice(
    ActiveVoice& activeVoice)
    noexcept
{
    if (!activeVoice.voice)
    {
        return;
    }


    activeVoice.voice->
        DestroyVoice();

    activeVoice.voice =
        nullptr;
}

void AudioSystem::
DestroyAllSourceVoices()
noexcept
{
    for (ActiveVoice& activeVoice :
        m_activeVoices)
    {
        DestroySourceVoice(
            activeVoice
        );
    }
}

bool AudioSystem::PlayOneShot(
    const AudioClip& clip,
    float volume)
{
    if (!IsInitialized())
    {
        ENGINE_DEBUG_LOG(
            "[Audio] PlayOneShot called "
            "before initialization.\n"
        );

        return false;
    }


    if (!clip.IsValid())
    {
        ENGINE_DEBUG_LOG(
            "[Audio] PlayOneShot received "
            "an invalid AudioClip.\n"
        );

        return false;
    }


    if (!std::isfinite(volume))
    {
        return false;
    }


    const std::size_t audioByteCount =
        clip.GetAudioByteCount();


    if (audioByteCount == 0 ||
        audioByteCount >
        static_cast<std::size_t>(
            (std::numeric_limits<
                UINT32
            >::max)()
            ))
    {
        ENGINE_DEBUG_LOG(
            "[Audio] AudioClip buffer "
            "size is invalid.\n"
        );

        return false;
    }


    //
    // 이번 frame 전에 완료된 voice가 있다면
    // 우선 회수한다.
    //
    Update();


    ActiveVoice* activeVoice =
        FindFreeVoiceSlot();


    if (!activeVoice)
    {
        ENGINE_DEBUG_LOG(
            "[Audio] Maximum active "
            "SFX voice count reached.\n"
        );

        return false;
    }


    IXAudio2SourceVoice*
        sourceVoice = nullptr;


    HRESULT hr =
        CreateRoutedSourceVoice(
            &sourceVoice,
            clip.GetFormat(),
            AudioCategory::Sfx
        );


    if (FAILED(hr) ||
        !sourceVoice)
    {
        AUDIO_DEBUG_LOG(
            "CreateSourceVoice",
            hr
        );

        return false;
    }


    const float clampedVolume =
        std::clamp(
            volume,
            0.0f,
            1.0f
        );


    hr =
        sourceVoice->
        SetVolume(
            clampedVolume
        );


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SetVolume",
            hr
        );

        sourceVoice->
            DestroyVoice();

        return false;
    }


    XAUDIO2_BUFFER
        buffer{};


    buffer.Flags =
        XAUDIO2_END_OF_STREAM;

    buffer.AudioBytes =
        static_cast<UINT32>(
            audioByteCount
            );

    buffer.pAudioData =
        reinterpret_cast<
        const BYTE*
        >(
            clip.GetAudioData()
            );


    hr =
        sourceVoice->
        SubmitSourceBuffer(
            &buffer
        );


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SubmitSourceBuffer",
            hr
        );

        sourceVoice->
            DestroyVoice();

        return false;
    }


    hr =
        sourceVoice->
        Start(
            0
        );


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SourceVoice::Start",
            hr
        );

        sourceVoice->
            DestroyVoice();

        return false;
    }


    activeVoice->voice =
        sourceVoice;


    return true;
}

std::size_t
AudioSystem::GetActiveVoiceCount()
const noexcept
{
    std::size_t count =
        0;


    for (const ActiveVoice& activeVoice :
        m_activeVoices)
    {
        if (activeVoice.voice)
        {
            ++count;
        }
    }


    return count;
}

AudioPlaybackHandle
AudioSystem::AllocatePlaybackHandle()
{
    static std::atomic<
        AudioPlaybackHandle::ValueType
    >
        nextValue{ 1 };


    const auto value =
        nextValue.fetch_add(
            1,
            std::memory_order_relaxed
        );


    if (value ==
        AudioPlaybackHandle::
        InvalidValue)
    {
        return {};
    }


    return
        AudioPlaybackHandle
    {
        value
    };
}

AudioSystem::PersistentVoice*
AudioSystem::FindFreePersistentVoice()
noexcept
{
    for (PersistentVoice& voice :
        m_persistentVoices)
    {
        if (!voice.voice)
        {
            return &voice;
        }
    }

    return nullptr;
}

AudioSystem::PersistentVoice*
AudioSystem::FindPersistentVoice(
    AudioPlaybackHandle handle)
    noexcept
{
    if (!handle.IsValid())
    {
        return nullptr;
    }


    for (PersistentVoice& voice :
        m_persistentVoices)
    {
        if (!voice.voice)
        {
            continue;
        }


        if (voice.handle ==
            handle)
        {
            return &voice;
        }
    }


    return nullptr;
}

const AudioSystem::PersistentVoice*
AudioSystem::FindPersistentVoice(
    AudioPlaybackHandle handle)
    const noexcept
{
    if (!handle.IsValid())
    {
        return nullptr;
    }


    for (const PersistentVoice& voice :
        m_persistentVoices)
    {
        if (!voice.voice)
        {
            continue;
        }


        if (voice.handle ==
            handle)
        {
            return &voice;
        }
    }


    return nullptr;
}

void AudioSystem::DestroyPersistentVoice(
    PersistentVoice& persistentVoice)
    noexcept
{
    if (persistentVoice.voice)
    {
        //
        // 더 이상 source data를 소비하지 않도록
        // 먼저 정지.
        //
        persistentVoice.voice->
            Stop(0);


        persistentVoice.voice->
            DestroyVoice();


        persistentVoice.voice =
            nullptr;
    }


    persistentVoice.handle =
        AudioPlaybackHandle{};

    persistentVoice.paused =
        false;
}

void AudioSystem::
DestroyAllPersistentVoices()
noexcept
{
    for (PersistentVoice& voice :
        m_persistentVoices)
    {
        DestroyPersistentVoice(
            voice
        );
    }
}

AudioPlaybackHandle
AudioSystem::PlayLoop(
    const AudioClip& clip,
    float volume)
{
    return
        PlayLoop(
            clip,
            AudioCategory::Music,
            volume
        );
}

AudioPlaybackHandle
AudioSystem::PlayLoop(
    const AudioClip& clip,
    AudioCategory category,
    float volume)
{
    if (!IsInitialized())
    {
        ENGINE_DEBUG_LOG(
            "[Audio] PlayLoop called "
            "before initialization.\n"
        );

        return {};
    }


    if (!clip.IsValid())
    {
        ENGINE_DEBUG_LOG(
            "[Audio] PlayLoop received "
            "an invalid AudioClip.\n"
        );

        return {};
    }


    if (!std::isfinite(volume))
    {
        return {};
    }


    const std::size_t
        audioByteCount =
        clip.GetAudioByteCount();


    if (audioByteCount == 0 ||
        audioByteCount >
        static_cast<std::size_t>(
            (std::numeric_limits<
                UINT32
            >::max)()
            ))
    {
        return {};
    }


    PersistentVoice* slot =
        FindFreePersistentVoice();


    if (!slot)
    {
        ENGINE_DEBUG_LOG(
            "[Audio] Maximum persistent "
            "voice count reached.\n"
        );

        return {};
    }


    IXAudio2SourceVoice*
        sourceVoice = nullptr;


    HRESULT hr =
        CreateRoutedSourceVoice(
            &sourceVoice,
            clip.GetFormat(),
            category
        );


    if (FAILED(hr) ||
        !sourceVoice)
    {
        AUDIO_DEBUG_LOG(
            "CreateSourceVoice(loop)",
            hr
        );

        return {};
    }


    const float clampedVolume =
        std::clamp(
            volume,
            0.0f,
            1.0f
        );


    hr =
        sourceVoice->
        SetVolume(
            clampedVolume
        );


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SetVolume(loop)",
            hr
        );

        sourceVoice->
            DestroyVoice();

        return {};
    }


    XAUDIO2_BUFFER buffer{};


    buffer.Flags =
        XAUDIO2_END_OF_STREAM;

    buffer.AudioBytes =
        static_cast<UINT32>(
            audioByteCount
            );

    buffer.pAudioData =
        reinterpret_cast<
        const BYTE*
        >(
            clip.GetAudioData()
            );


    //
    // LoopBegin = 0
    // LoopLength = 0
    //
    // + infinite LoopCount
    //
    // => 전체 sample을 무한 반복.
    //
    buffer.LoopBegin =
        0;

    buffer.LoopLength =
        0;

    buffer.LoopCount =
        XAUDIO2_LOOP_INFINITE;


    hr =
        sourceVoice->
        SubmitSourceBuffer(
            &buffer
        );


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SubmitSourceBuffer(loop)",
            hr
        );

        sourceVoice->
            DestroyVoice();

        return {};
    }


    hr =
        sourceVoice->Start(0);


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SourceVoice::Start(loop)",
            hr
        );

        sourceVoice->
            DestroyVoice();

        return {};
    }


    const AudioPlaybackHandle
        handle =
        AllocatePlaybackHandle();


    if (!handle.IsValid())
    {
        sourceVoice->
            DestroyVoice();

        return {};
    }


    slot->handle =
        handle;

    slot->voice =
        sourceVoice;

    slot->paused =
        false;


    return handle;
}

bool AudioSystem::Pause(
    AudioPlaybackHandle handle)
{
    PersistentVoice* playback =
        FindPersistentVoice(
            handle
        );


    if (!playback ||
        playback->paused)
    {
        return false;
    }


    const HRESULT hr =
        playback->voice->
        Stop(0);


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SourceVoice::Stop(pause)",
            hr
        );

        return false;
    }


    playback->paused =
        true;


    return true;
}

bool AudioSystem::Resume(
    AudioPlaybackHandle handle)
{
    PersistentVoice* playback =
        FindPersistentVoice(
            handle
        );


    if (!playback ||
        !playback->paused)
    {
        return false;
    }


    const HRESULT hr =
        playback->voice->
        Start(0);


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SourceVoice::Start(resume)",
            hr
        );

        return false;
    }


    playback->paused =
        false;


    return true;
}

bool AudioSystem::Stop(
    AudioPlaybackHandle handle)
{
    PersistentVoice* playback =
        FindPersistentVoice(
            handle
        );


    if (!playback)
    {
        return false;
    }


    DestroyPersistentVoice(
        *playback
    );


    return true;
}

bool AudioSystem::IsPlaybackValid(
    AudioPlaybackHandle handle)
    const noexcept
{
    return
        FindPersistentVoice(
            handle
        ) != nullptr;
}

bool AudioSystem::IsPaused(
    AudioPlaybackHandle handle)
    const noexcept
{
    const PersistentVoice* playback =
        FindPersistentVoice(
            handle
        );


    if (!playback)
    {
        return false;
    }


    return
        playback->paused;
}

std::size_t
AudioSystem::GetPersistentVoiceCount()
const noexcept
{
    std::size_t count =
        0;


    for (const PersistentVoice& voice :
        m_persistentVoices)
    {
        if (voice.voice)
        {
            ++count;
        }
    }


    return count;
}

IXAudio2Voice*
AudioSystem::GetCategoryOutputVoice(
    AudioCategory category)
    const noexcept
{
    switch (category)
    {
    case AudioCategory::Sfx:
        return m_sfxSubmixVoice;

    case AudioCategory::Music:
        return m_musicSubmixVoice;

    default:
        return nullptr;
    }
}

HRESULT
AudioSystem::CreateRoutedSourceVoice(
    IXAudio2SourceVoice** outVoice,
    const WAVEFORMATEX& format,
    AudioCategory category) noexcept
{
    if (!outVoice)
    {
        return E_POINTER;
    }

    *outVoice =
        nullptr;


    if (!m_xaudio2)
    {
        return E_FAIL;
    }


    IXAudio2Voice* outputVoice =
        GetCategoryOutputVoice(
            category
        );


    if (!outputVoice)
    {
        return E_FAIL;
    }


    XAUDIO2_SEND_DESCRIPTOR
        sendDescriptor{};

    sendDescriptor.Flags =
        0;

    sendDescriptor.pOutputVoice =
        outputVoice;


    XAUDIO2_VOICE_SENDS
        sendList{};

    sendList.SendCount =
        1;

    sendList.pSends =
        &sendDescriptor;


    return
        m_xaudio2->
        CreateSourceVoice(
            outVoice,
            &format,
            0,
            XAUDIO2_DEFAULT_FREQ_RATIO,
            nullptr,
            &sendList,
            nullptr
        );
}

bool AudioSystem::SetMasterVolume(
    float volume)
{
    if (!IsInitialized() ||
        !std::isfinite(volume))
    {
        return false;
    }


    const float clamped =
        std::clamp(
            volume,
            0.0f,
            1.0f
        );


    const HRESULT hr =
        m_masterVoice->
        SetVolume(
            clamped
        );


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SetVolume(Master)",
            hr
        );

        return false;
    }


    m_masterVolume =
        clamped;


    return true;
}

bool AudioSystem::SetSfxVolume(
    float volume)
{
    if (!IsInitialized() ||
        !std::isfinite(volume))
    {
        return false;
    }


    const float clamped =
        std::clamp(
            volume,
            0.0f,
            1.0f
        );


    const HRESULT hr =
        m_sfxSubmixVoice->
        SetVolume(
            clamped
        );


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SetVolume(SFX)",
            hr
        );

        return false;
    }


    m_sfxVolume =
        clamped;


    return true;
}

bool AudioSystem::SetMusicVolume(
    float volume)
{
    if (!IsInitialized() ||
        !std::isfinite(volume))
    {
        return false;
    }


    const float clamped =
        std::clamp(
            volume,
            0.0f,
            1.0f
        );


    const HRESULT hr =
        m_musicSubmixVoice->
        SetVolume(
            clamped
        );


    if (FAILED(hr))
    {
        AUDIO_DEBUG_LOG(
            "SetVolume(Music)",
            hr
        );

        return false;
    }


    m_musicVolume =
        clamped;


    return true;
}

float AudioSystem::GetMasterVolume()
const noexcept
{
    return m_masterVolume;
}


float AudioSystem::GetSfxVolume()
const noexcept
{
    return m_sfxVolume;
}


float AudioSystem::GetMusicVolume()
const noexcept
{
    return m_musicVolume;
}

AudioPlaybackHandle
AudioSystem::PlayMusic(
    const AudioClip& clip,
    float volume)
{
    return
        PlayLoop(
            clip,
            AudioCategory::Music,
            volume
        );
}