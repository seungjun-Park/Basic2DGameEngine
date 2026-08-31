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

    static void Reset();

private:
    static std::array<bool, 256> s_currentKeys;
    static std::array<bool, 256> s_previousKeys;
};