#pragma once
#include "pch.h"

inline void PrintExceptionStack(const std::exception& aException, int aDepth = 0)
{
    const std::string message(aDepth * 2, ' ');
    std::cout << message << aException.what() << std::endl;
    try
    {
        std::rethrow_if_nested(aException);
    }
    catch (const std::exception& exception)
    {
        PrintExceptionStack(exception, aDepth + 1);
    }
    catch (...)
    {
    }
}


