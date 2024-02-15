#include "pch.h"
#include "GraphicsEngine.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

void GuardedMain()
{
    const HINSTANCE hInstance = GetModuleHandleW(NULL);

    {
        WNDCLASSEXW wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = NULL;
        wcex.hCursor = NULL;
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = L"comrade-klick";
        wcex.hIconSm = NULL;
        RegisterClassExW(&wcex);
    }

    HWND hwnd{};
    {
        hwnd = CreateWindowExW(0L, L"comrade-klick",
            L"comrade-klick", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL,
            NULL, hInstance, NULL);

        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
        UpdateWindow(hwnd);
    }

    
    GraphicsEngine graphicsEngine;
    graphicsEngine.OnInitialize(hwnd);
    bool running = true;
    while (running)
    {
        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            running &= (msg.message != WM_QUIT);
        }
        graphicsEngine.OnUpdate();
        graphicsEngine.OnRender();
    }
    graphicsEngine.OnDestroy();
}