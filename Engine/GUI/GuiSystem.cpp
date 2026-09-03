#include "GuiSystem.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

GuiSystem::~GuiSystem()
{
    Shutdown();
}

bool GuiSystem::Initialize(
    HWND hwnd,
    ID3D11Device* device,
    ID3D11DeviceContext* context)
{
    if (m_initialized)
    {
        return true;
    }

    if (hwnd == nullptr ||
        device == nullptr ||
        context == nullptr)
    {
        return false;
    }

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    // Phase 14-A에서는 GUI layout persistence를
    // serialization contract에 포함시키지 않는다.
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(hwnd))
    {
        ImGui::DestroyContext();

        return false;
    }

    if (!ImGui_ImplDX11_Init(device, context))
    {
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        return false;
    }

    m_initialized = true;

    return true;
}

void GuiSystem::Shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();

    m_initialized = false;
}

void GuiSystem::BeginFrame()
{
    if (!m_initialized)
    {
        return;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
}

void GuiSystem::DrawFoundationWindow()
{
    if (!m_initialized)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    ImGui::Begin("Basic2DGameEngine - Phase 14-A");

    ImGui::Text("Dear ImGui integration active.");
    ImGui::Text("Win32 + DirectX 11");

    ImGui::Separator();

    ImGui::InputText(
        "Keyboard Capture Test",
        m_keyboardCaptureTest.data(),
        m_keyboardCaptureTest.size()
    );

    ImGui::Text(
        "FPS: %.1f",
        io.Framerate);

    ImGui::Text(
        "WantCaptureKeyboard: %s",
        io.WantCaptureKeyboard ? "true" : "false");

    ImGui::Text(
        "WantCaptureMouse: %s",
        io.WantCaptureMouse ? "true" : "false");

    ImGui::SliderFloat(
        "Foundation Test",
        &m_testValue,
        0.0f,
        1.0f);

    ImGui::End();
}

void GuiSystem::Render()
{
    if (!m_initialized)
    {
        return;
    }

    ImGui::Render();

    ImGui_ImplDX11_RenderDrawData(
        ImGui::GetDrawData());
}

bool GuiSystem::WantsCaptureKeyboard() const noexcept
{
    if (!m_initialized)
    {
        return false;
    }

    return
        ImGui::GetIO().WantCaptureKeyboard;
}

bool GuiSystem::WantsCaptureMouse() const noexcept
{
    if (!m_initialized)
    {
        return false;
    }

    return
        ImGui::GetIO().WantCaptureMouse;
}

