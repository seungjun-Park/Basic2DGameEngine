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
};