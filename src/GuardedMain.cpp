#include "pch.h"
#include "Camera.h"
#include "Clock.h"
#include "GraphicsEngine/GraphicsEngine.h"
#include "Window.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INPUT:
        {
            static RAWINPUT raw{};
            unsigned int size = sizeof(RAWINPUT);
            if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) != static_cast<unsigned int>(-1))
            {
                if (raw.header.dwType == RIM_TYPEMOUSE)
                {
                    globalWindow.mouse.delta.x += static_cast<FLOAT>(static_cast<INT16>(raw.data.mouse.lLastX));
                    globalWindow.mouse.delta.y += static_cast<FLOAT>(static_cast<INT16>(raw.data.mouse.lLastY));
                }
            }
            break;
        }
        case WM_KEYDOWN:
        {
            globalWindow.keyboard.w |= (wParam == 0x57);
            globalWindow.keyboard.a |= (wParam == 0x41);
            globalWindow.keyboard.s |= (wParam == 0x53);
            globalWindow.keyboard.d |= (wParam == 0x44);
            globalWindow.keyboard.e |= (wParam == 0x45);
            globalWindow.keyboard.q |= (wParam == 0x51);

            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            break;
        }
        case WM_KEYUP:
        {
            globalWindow.keyboard.w &= (wParam != 0x57);
            globalWindow.keyboard.a &= (wParam != 0x41);
            globalWindow.keyboard.s &= (wParam != 0x53);
            globalWindow.keyboard.d &= (wParam != 0x44);
            globalWindow.keyboard.e &= (wParam != 0x45);
            globalWindow.keyboard.q &= (wParam != 0x51);
            break;
        }
        case WM_SIZE:
        {
            globalWindow.size.x = static_cast<FLOAT>(GET_X_LPARAM(lParam));
            globalWindow.size.y = static_cast<FLOAT>(GET_Y_LPARAM(lParam));
            globalWindow.resize = globalWindow.size;
            globalWindow.aspectRatio = globalWindow.size.x / globalWindow.size.y;
            break;
        }
        case WM_MOUSEMOVE:
        {
            globalWindow.mouse.pos.x = static_cast<FLOAT>(GET_X_LPARAM(lParam));
            globalWindow.mouse.pos.y = static_cast<FLOAT>(GET_Y_LPARAM(lParam));
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_ACTIVATE:
        {
            if (wParam == WA_INACTIVE)
            {
                ShowCursor(TRUE);
            }
            else
            {
                RECT mouseClipRect{};
                mouseClipRect.left = static_cast<LONG>(globalWindow.size.x / 2.f);
                mouseClipRect.top = static_cast<LONG>(globalWindow.size.y / 2.f);
                mouseClipRect.right = static_cast<LONG>(globalWindow.size.x / 2.f);
                mouseClipRect.bottom = static_cast<LONG>(globalWindow.size.y / 2.f);
                ClipCursor(&mouseClipRect);
                ShowCursor(FALSE);
            }
            break;
        }
        default:
        {
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
    }
    return 0;
}

void GuardedMain()
{
    {
        const HINSTANCE hInstance = GetModuleHandleW(NULL);

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
    
        globalWindow.hwnd = CreateWindowExW(0L, L"comrade-klick",
            L"comrade-klick", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL,
            NULL, hInstance, NULL);
        ShowWindow(globalWindow.hwnd, SW_SHOWMAXIMIZED);
        UpdateWindow(globalWindow.hwnd);


        enum {
            HID_USAGE_PAGE_GENERIC = 0x01,
            HID_USAGE_GENERIC_MOUSE = 0x02
        };
        RAWINPUTDEVICE device = {};
        device.dwFlags = RIDEV_INPUTSINK;
        device.hwndTarget = globalWindow.hwnd;
        device.usUsage = HID_USAGE_GENERIC_MOUSE;
        device.usUsagePage = HID_USAGE_PAGE_GENERIC;
        ThrowIfFailed(RegisterRawInputDevices(&device, 1, sizeof(device)));
    }

    Camera camera;
    camera.OnInitialize();
    GraphicsEngine graphicsEngine;
    graphicsEngine.OnInitialize();
    Clock clock;
    clock.OnInitialize();
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
        clock.OnUpdate();
        const float deltaSeconds = clock.GetDeltaSeconds(), 
                    timeSeconds = clock.GetTimeSeconds();

        camera.OnUpdate(deltaSeconds);
        graphicsEngine.OnUpdate(deltaSeconds, timeSeconds, camera.ToViewMatrix(), camera.ToProjectionMatrix());
        globalWindow.resize = Vec2::Zero;
        globalWindow.mouse.delta = Vec2::Zero;
    }
    graphicsEngine.OnDestroy();
}