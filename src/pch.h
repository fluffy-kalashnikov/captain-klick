#pragma once
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <d3d.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <wrl.h>
#include <stdexcept>
#include <format>
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

inline std::string ToString(HRESULT aCode)
{
	std::string error;
	char* outString = NULL;
	DWORD outSize = 0;
	{
		DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS;
		LPVOID lpSource = NULL;
		DWORD dwMessageId = aCode;
		DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
		LPSTR lpBuffer = reinterpret_cast<LPSTR>(&outString);
		DWORD dwSize = 0;
		va_list* arguments = NULL;

		outSize = FormatMessageA(dwFlags, lpSource,
			dwMessageId, dwLanguageId, lpBuffer, dwSize, arguments);
	}
	if (outSize > 0)
	{
		error = std::string(outString, outSize);
	}
	LocalFree(outString);
	return error;
}

inline void ThrowIfFailed(HRESULT aResult)
{
	if (FAILED(aResult))
	{
		throw std::runtime_error(ToString(aResult));
	}
}