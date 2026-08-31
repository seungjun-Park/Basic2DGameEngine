#pragma once

#include <Windows.h>

class WinWindow
{
public:
    WinWindow() = default;
    ~WinWindow();

    WinWindow(const WinWindow&) = delete;
    WinWindow& operator=(const WinWindow&) = delete;

    bool Initialize(
        HINSTANCE hInstance,
        int width,
        int height,
        const wchar_t* title
    );

    bool ProcessMessages();

    HWND GetHandle() const
    {
        return m_hwnd;
    }

    int GetWidth() const
    {
        return m_width;
    }

    int GetHeight() const
    {
        return m_height;
    }

    bool IsActive() const
    {
        return m_isActive;
    }

    bool IsMinimized() const
    {
        return m_isMinimized;
    }

    bool ConsumeResize(
        int& width,
        int& height
    );

private:
    static LRESULT CALLBACK WindowProc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    bool RegisterWindowClass(HINSTANCE hInstance);

private:
    HWND m_hwnd = nullptr;

    HINSTANCE m_hInstance = nullptr;

    int m_width = 0;
    int m_height = 0;

    const wchar_t* m_title = L"Dobi2D";
    const wchar_t* m_className = L"Dobi2DWindowClass";

    bool m_isActive = true;
    bool m_isMinimized = false;

    bool m_hasPendingResize = false;

    int m_pendingWidth = 0;
    int m_pendingHeight = 0;
};