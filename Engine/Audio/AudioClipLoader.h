#pragma once

#include <memory>
#include <string>


class AudioClip;


class AudioClipLoader final
{
public:

    static std::unique_ptr<AudioClip>
        Load(
            const std::wstring& path
        );
};