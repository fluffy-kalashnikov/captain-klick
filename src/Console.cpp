#include "pch.h"
#include "Console.h"

Console::Console()
{
    FILE* consoleOut;
    FILE* consoleErr;
    AllocConsole();
    freopen_s(&consoleOut, "CONOUT$", "w", stdout);
    freopen_s(&consoleErr, "CONOUT$", "w", stderr);
    setvbuf(consoleOut, nullptr, _IONBF, 1024);
}

Console::~Console()
{
    FreeConsole();
}