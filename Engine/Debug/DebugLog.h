#pragma once

#if defined(_DEBUG)

#include <Windows.h>

namespace DebugLog
{
    void Write(
        const char* message
    ) noexcept;

    void Write(
        const wchar_t* message
    ) noexcept;

    void LogAnimationClipError(
        const char* message
    ) noexcept;
    void LogLoadFailure(
        const char* reason
    ) noexcept;

    void LogAudioError(
        const char* operation,
        HRESULT hr
    ) noexcept;

    void LogSceneError(
        const char* message
    ) noexcept;

    void LogTileMapError(
        const char* message
    ) noexcept;

    void LogTilesetError(
        const char* message
    ) noexcept;

}

#define ENGINE_DEBUG_LOG(message) \
    ::DebugLog::Write(message)

#define ANIMATIONCLIP_DEBUG_LOG(message) \
    ::DebugLog::LogAnimationClipError(message)

#define AUDIOLOADER_DEBUG_LOG(reason) \
    ::DebugLog::LogLoadFailure(reason)

#define AUDIO_DEBUG_LOG(operation, hr) \
    ::DebugLog::LogAudioError(operation, hr)

#define SCENE_DEBUG_LOG(message) \
    ::DebugLog::LogSceneError(message)

#define TILEMAP_DEBUG_LOG(message) \
    ::DebugLog::LogTileMapError(message)

#define TILESET_DEBUG_LOG(message) \
    ::DebugLog::LogTilesetError(message)

#else

#define ENGINE_DEBUG_LOG(message) \
    ((void)0)

#define ANIMATIONCLIP_DEBUG_LOG(message) \
    ((void)0)

#define AUDIOLOADER_DEBUG_LOG(message) \
    ((void)0)

#define AUDIO_DEBUG_LOG(operation, hr) \
    ((void)0)

#define SCENE_DEBUG_LOG(message) \
    ((void)0)

#define TILEMAP_DEBUG_LOG(message) \
    ((void)0)

#define TILESET_DEBUG_LOG(message) \
    ((void)0)

#endif