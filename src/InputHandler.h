#pragma once
#include "Math/Vec2.h"
#include "pch.h"

class GraphicsSwapChain;
inline class InputHandler
{
public:
	struct Keyboard
	{
		bool w{}, 
			 a{}, 
			 s{}, 
			 d{}, 
			 e{}, 
			 q{},
			space{};
	} keyboard;
	struct Mouse
	{
		Vec2 delta;
		Vec2 pos;
	} mouse;
	void WndProcHandler(GraphicsSwapChain& aGraphicsSwapChain, UINT message, WPARAM wParam, LPARAM lParam);
	void EndFrame();
} globalInputHandler;

