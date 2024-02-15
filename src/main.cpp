#include "pch.h"
#include "Console.h"
#include <iostream>

void PrintNestedException(const std::exception& aException, int aDepth = 0)
{
    const std::string message(aDepth * 2, ' ');
    std::cout << message << aException.what() << std::endl;
    try
    {
        std::rethrow_if_nested(aException);
    }
    catch (const std::exception& exception)
    {
        PrintNestedException(exception, aDepth + 1);
    }
    catch (...)
    {
    }
}



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
        PrintNestedException(exception);
        __debugbreak();
    }
}