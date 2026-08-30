#pragma once

#include <Windows.h>

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual bool Initialize(
        HWND hwnd,
        int width,
        int height
    ) = 0;

    virtual void BeginFrame() = 0;

    virtual void EndFrame() = 0;

    virtual void Clear() = 0;
};
