#include "DebugLog.h"

#if defined(_DEBUG)

#include <stdio.h>

namespace DebugLog
{
    void Write(
        const char* message
    ) noexcept
    {
        if (!message)
        {
            return;
        }

        ::OutputDebugStringA(
            message
        );
    }

    void Write(
        const wchar_t* message
    ) noexcept
    {
        if (!message)
        {
            return;
        }

        ::OutputDebugStringW(
            message
        );
    }


    void LogAnimationClipError(
        const char* message
    ) noexcept
    {
        OutputDebugStringA(
            "[AnimationClipLoader] "
        );

        OutputDebugStringA(
            message
        );

        OutputDebugStringA(
            "\n"
        );
    }

    void LogLoadFailure(
        const char* reason
    ) noexcept
    {
        OutputDebugStringA(
            "[Audio] WAV load failed: "
        );

        OutputDebugStringA(
            reason
        );

        OutputDebugStringA(
            "\n"
        );
    }

    void LogAudioError(
        const char* operation,
        HRESULT hr
    ) noexcept
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

    void LogSceneError(
        const char* message
    ) noexcept
    {
        OutputDebugStringA(
            "[SceneSerializer] "
        );

        OutputDebugStringA(
            message
        );

        OutputDebugStringA(
            "\n"
        );
    }

    void LogTileMapError(
        const char* message
    ) noexcept
    {
        OutputDebugStringA(
            "[TileMapLoader] "
        );

        OutputDebugStringA(
            message
        );

        OutputDebugStringA(
            "\n"
        );
    }

    void LogTilesetError(
        const char* message
    ) noexcept
    {
        OutputDebugStringA(
            "[TilesetLoader] "
        );

        OutputDebugStringA(
            message
        );

        OutputDebugStringA(
            "\n"
        );
    }
}

#endif