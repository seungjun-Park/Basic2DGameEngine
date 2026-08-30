#include "WinInput.h"

#include <Windows.h>

std::array<bool, 256> WinInput::s_currentKeys{};
std::array<bool, 256> WinInput::s_previousKeys{};

void WinInput::Initialize()
{
    s_currentKeys.fill(false);
    s_previousKeys.fill(false);
}

void WinInput::Update()
{
    s_previousKeys = s_currentKeys;

    for (int key = 0; key < 256; ++key)
    {
        SHORT state =
            GetAsyncKeyState(key);

        s_currentKeys[key] =
            (state & 0x8000) != 0;
    }
}

bool WinInput::IsKeyDown(int key)
{
    return s_currentKeys[key];
}

bool WinInput::IsKeyPressed(int key)
{
    return
        s_currentKeys[key] &&
        !s_previousKeys[key];
}

bool WinInput::IsKeyReleased(int key)
{
    return
        !s_currentKeys[key] &&
        s_previousKeys[key];
}