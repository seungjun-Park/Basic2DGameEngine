#include "Core/Application.h"

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int)
{
    Application application;

    if (!application.Initialize(
        hInstance))
    {
        MessageBoxW(
            nullptr,
            L"Application initialization failed.",
            L"Test",
            MB_OK | MB_ICONERROR
        );

        return -1;
    }

    return application.Run();
}