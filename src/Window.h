#pragma once
#include "Math/Vec2.h"

struct HWND__;
using HWND = HWND__*;

inline struct Window
{
	HWND hwnd{};
	Vec2 size;
	Vec2 resize;
	float aspectRatio{};
	struct Keyboard
	{
		bool w{}, 
			 a{}, 
			 s{}, 
			 d{}, 
			 e{}, 
			 q{};
	} keyboard;
	struct Mouse
	{
		Vec2 delta;
		Vec2 pos;
	} mouse;
} globalWindow;

