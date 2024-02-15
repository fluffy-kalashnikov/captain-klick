#pragma once

class Console
{
public:
	Console();
	Console(const Console&) = delete;
	~Console();
	Console& operator=(const Console&) = delete;
};