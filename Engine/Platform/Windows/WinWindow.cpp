#include "WinWindow.h"

#include <imgui.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT
ImGui_ImplWin32_WndProcHandler(
    HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);


WinWindow::~WinWindow()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    if (m_hInstance)
    {
        UnregisterClassW(
            m_className,
            m_hInstance
        );
    }
}

bool WinWindow::Initialize(
    HINSTANCE hInstance,
    int width,
    int height,
    const wchar_t* title)
{
    m_hInstance = hInstance;
    m_width = width;
    m_height = height;
    m_title = title;

    if (!RegisterWindowClass(hInstance))
    {
        return false;
    }

    RECT rect
    {
        0,
        0,
        width,
        height
    };

    // 클라이언트 영역이 width x height가 되도록
    // 윈도우 외곽 크기를 조정한다.
    AdjustWindowRect(
        &rect,
        WS_OVERLAPPEDWINDOW,
        FALSE
    );

    int windowWidth =
        rect.right - rect.left;

    int windowHeight =
        rect.bottom - rect.top;

    m_hwnd = CreateWindowExW(
        0,
        m_className,
        m_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowWidth,
        windowHeight,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (!m_hwnd)
    {
        return false;
    }

    ShowWindow(
        m_hwnd,
        SW_SHOW
    );

    UpdateWindow(m_hwnd);

    return true;
}


bool WinWindow::ProcessMessages()
{
    MSG message{};

    while (PeekMessageW(
        &message,
        nullptr,
        0,
        0,
        PM_REMOVE))
    {
        if (message.message == WM_QUIT)
        {
            return false;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return true;
}

bool WinWindow::ConsumeResize(
    int& width,
    int& height)
{
    if (!m_hasPendingResize)
    {
        return false;
    }

    width =
        m_pendingWidth;

    height =
        m_pendingHeight;

    m_hasPendingResize =
        false;

    return true;
}

bool WinWindow::RegisterWindowClass(
    HINSTANCE hInstance)
{
    WNDCLASSEXW wc{};

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(
        nullptr,
        IDC_ARROW
    );
    wc.hbrBackground =
        static_cast<HBRUSH>(
            GetStockObject(BLACK_BRUSH)
            );
    wc.lpszClassName = m_className;

    ATOM result = RegisterClassExW(&wc);

    if (result == 0)
    {
        DWORD error = GetLastError();

        // 이미 등록된 경우 성공으로 처리
        if (error != ERROR_CLASS_ALREADY_EXISTS)
        {
            return false;
        }
    }

    return true;
}

LRESULT CALLBACK WinWindow::WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(
        hwnd,
        message,
        wParam,
        lParam))
    {
        return 1;
    }

    WinWindow* window = nullptr;

    if (message == WM_NCCREATE)
    {
        auto* createStruct =
            reinterpret_cast<CREATESTRUCTW*>(
                lParam
                );

        window =
            static_cast<WinWindow*>(
                createStruct->lpCreateParams
                );

        SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(window)
        );

        window->m_hwnd = hwnd;
    }
    else
    {
        window =
            reinterpret_cast<WinWindow*>(
                GetWindowLongPtrW(
                    hwnd,
                    GWLP_USERDATA
                )
                );
    }

    switch (message)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_ACTIVATEAPP:
    {
        if (window)
        {
            window->m_isActive =
                (wParam != FALSE);
        }

        return 0;
    }
    case WM_SIZE:
    {
        if (!window)
        {
            return 0;
        }

        if (wParam == SIZE_MINIMIZED)
        {
            window->m_isMinimized = true;

            return 0;
        }

        window->m_isMinimized = false;

        const int width =
            static_cast<int>(
                LOWORD(lParam)
                );

        const int height =
            static_cast<int>(
                HIWORD(lParam)
                );

        if (width <= 0 ||
            height <= 0)
        {
            return 0;
        }

        window->m_width = width;
        window->m_height = height;

        window->m_pendingWidth = width;
        window->m_pendingHeight = height;

        window->m_hasPendingResize = true;

        return 0;
    }
    }

    return DefWindowProcW(
        hwnd,
        message,
        wParam,
        lParam
    );
}