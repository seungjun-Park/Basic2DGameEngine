#pragma once

#include <array>

class WinInput
{
public:
    static void Initialize();
    static void Update();

    static bool IsKeyDown(int key);
    static bool IsKeyPressed(int key);
    static bool IsKeyReleased(int key);

    //
    // Engine/global input.
    // GUI capture 상태와 무관한 physical snapshot.
    //
    static bool IsRawKeyDown(int key);
    static bool IsRawKeyPressed(int key);
    static bool IsRawKeyReleased(int key);

    static void SetCaptureState(
        bool keyboardCaptured,
        bool mouseCaptured);

    static void Reset();

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