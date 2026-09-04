#include "WinInput.h"

#include <Windows.h>

std::array<bool, 256> WinInput::s_currentKeys{};
std::array<bool, 256> WinInput::s_previousKeys{};

bool WinInput::s_keyboardCaptured = false;
bool WinInput::s_mouseCaptured = false;

void WinInput::Initialize()
{
    s_currentKeys.fill(false);
    s_previousKeys.fill(false);

    s_keyboardCaptured = false;
    s_mouseCaptured = false;
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

void WinInput::Reset()
{
    s_currentKeys.fill(false);
    s_previousKeys.fill(false);
}

void WinInput::SetCaptureState(
    bool keyboardCaptured,
    bool mouseCaptured)
{
    s_keyboardCaptured =
        keyboardCaptured;

    s_mouseCaptured =
        mouseCaptured;
}

bool WinInput::IsKeyDown(
    int key)
{
    if (!IsValidKey(key))
    {
        return false;
    }

    if (IsCapturedForKey(key))
    {
        return false;
    }

    return IsRawKeyDown(key);
}

bool WinInput::IsKeyPressed(
    int key)
{
    if (!IsValidKey(key))
    {
        return false;
    }

    if (IsCapturedForKey(key))
    {
        return false;
    }

    return IsRawKeyPressed(key);
}

bool WinInput::IsKeyReleased(
    int key)
{
    if (!IsValidKey(key))
    {
        return false;
    }

    if (IsCapturedForKey(key))
    {
        return false;
    }

    return IsRawKeyReleased(key);
}

bool WinInput::IsRawKeyDown(
    int key)
{
    if (!IsValidKey(key))
    {
        return false;
    }

    return
        s_currentKeys[
            static_cast<std::size_t>(key)
        ];
}

bool WinInput::IsRawKeyPressed(
    int key)
{
    if (!IsValidKey(key))
    {
        return false;
    }

    const std::size_t index =
        static_cast<std::size_t>(key);

    return
        s_currentKeys[index] &&
        !s_previousKeys[index];
}

bool WinInput::IsRawKeyReleased(
    int key)
{
    if (!IsValidKey(key))
    {
        return false;
    }

    const std::size_t index =
        static_cast<std::size_t>(key);

    return
        !s_currentKeys[index] &&
        s_previousKeys[index];
}

bool WinInput::IsMouseButtonKey(
    int key) noexcept
{
    switch (key)
    {
    case VK_LBUTTON:
    case VK_RBUTTON:
    case VK_MBUTTON:
    case VK_XBUTTON1:
    case VK_XBUTTON2:
        return true;

    default:
        return false;
    }
}

bool WinInput::IsCapturedForKey(
    int key) noexcept
{
    if (IsMouseButtonKey(key))
    {
        return s_mouseCaptured;
    }

    return s_keyboardCaptured;
}

bool WinInput::IsValidKey(
    int key) noexcept
{
    return
        key >= 0 &&
        key < 256;
}
