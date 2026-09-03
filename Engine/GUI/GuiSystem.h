#pragma once

#include <Windows.h>
#include <array>

struct ID3D11Device;
struct ID3D11DeviceContext;

class GuiSystem
{
public:
    GuiSystem() = default;
    ~GuiSystem();

    GuiSystem(const GuiSystem&) = delete;
    GuiSystem& operator=(const GuiSystem&) = delete;

    bool Initialize(
        HWND hwnd,
        ID3D11Device* device,
        ID3D11DeviceContext* context);

    void Shutdown();

    void BeginFrame();

    void DrawFoundationWindow();
    void Render();

    [[nodiscard]]
    bool IsInitialized() const noexcept
    {
        return m_initialized;
    }

    [[nodiscard]]
    bool WantsCaptureKeyboard() const noexcept;

    [[nodiscard]]
    bool WantsCaptureMouse() const noexcept;

private:
    bool m_initialized = false;

    float m_testValue = 0.5f;

    std::array<char, 128>
        m_keyboardCaptureTest{};
};
