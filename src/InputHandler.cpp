#include "InputHandler.h"
#include "GraphicsEngine/GraphicsSwapChain.h"

void InputHandler::WndProcHandler(GraphicsSwapChain& aGraphicsSwapChain, UINT message, WPARAM wParam, LPARAM lParam)
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
					mouse.delta.x += static_cast<FLOAT>(static_cast<INT16>(raw.data.mouse.lLastX));
					mouse.delta.y += static_cast<FLOAT>(static_cast<INT16>(raw.data.mouse.lLastY));
				}
			}
			break;
		}
		case WM_KEYDOWN:
		{
			keyboard.w |= (wParam == 0x57);
			keyboard.a |= (wParam == 0x41);
			keyboard.s |= (wParam == 0x53);
			keyboard.d |= (wParam == 0x44);
			keyboard.e |= (wParam == 0x45);
			keyboard.q |= (wParam == 0x51);
			keyboard.space |= (wParam == VK_SPACE);

			if (wParam == VK_ESCAPE)
			{
				PostQuitMessage(0);
			}
			break;
		}
		case WM_KEYUP:
		{
			keyboard.w &= (wParam != 0x57);
			keyboard.a &= (wParam != 0x41);
			keyboard.s &= (wParam != 0x53);
			keyboard.d &= (wParam != 0x44);
			keyboard.e &= (wParam != 0x45);
			keyboard.q &= (wParam != 0x51);
			keyboard.space &= (wParam != VK_SPACE);
			break;
		}
		case WM_MOUSEMOVE:
		{
			mouse.pos.x = static_cast<FLOAT>(GET_X_LPARAM(lParam));
			mouse.pos.y = static_cast<FLOAT>(GET_Y_LPARAM(lParam));
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
				UINT width = aGraphicsSwapChain.GetWidth(), 
					 height = aGraphicsSwapChain.GetHeight();
				LONG x = width / 2, 
					 y = height / 2;

				RECT mouseClipRect{};
				mouseClipRect.left = x;
				mouseClipRect.right = x;
				mouseClipRect.top = y;
				mouseClipRect.bottom = y;
				ClipCursor(&mouseClipRect);
				ShowCursor(FALSE);
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

void InputHandler::EndFrame()
{
	mouse.delta = {};
}
