#include "pch.h"
#include "Console.h"
#include "Exception.h"


int APIENTRY wWinMain(_In_ HINSTANCE /*hInstance*/,
                     _In_opt_ HINSTANCE /*hPrevInstance*/,
                     _In_ LPWSTR    /*lpCmdLine*/,
                     _In_ int       /*nCmdShow*/)
{
    Console console;

    try
    {
        void GuardedMain();
        GuardedMain();
    }
    catch (std::exception& exception)
    {
        PrintExceptionStack(exception);
        __debugbreak();
    }
}