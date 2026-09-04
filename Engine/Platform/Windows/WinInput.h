#pragma once

#include <array>

class WinInput
{
public:
    static void Initialize();
    static void Update();

    static void Reset();

    static void SetCaptureState(
        bool keyboardCaptured,
        bool mouseCaptured);

    static bool IsKeyDown(int key);
    static bool IsKeyPressed(int key);
    static bool IsKeyReleased(int key);

    static bool IsRawKeyDown(int key);
    static bool IsRawKeyPressed(int key);
    static bool IsRawKeyReleased(int key);

private:
    static bool IsValidKey(
        int key) noexcept;

    static bool IsMouseButtonKey(
        int key) noexcept;

    static bool IsCapturedForKey(
        int key) noexcept;

private:
    static std::array<bool, 256> s_currentKeys;
    static std::array<bool, 256> s_previousKeys;

    static bool s_keyboardCaptured;
    static bool s_mouseCaptured;
};