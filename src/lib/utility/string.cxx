// SPDX-License-Identifier: LGPL-3.0+

#include "./string.hxx"

#include <Windows.h>

#include <system_error>

#ifndef HAVE_STRNDUP
char* strndup(const char* str, size_t size)
{
	size_t len = strnlen_s(str, size);
	char* res = static_cast<char*>(malloc(len + 1));
	if (res) {
		strncpy(res, str, len);
		res[len] = '\0';
	}
	return res;
}
#endif

std::string wm_sensors::windowsErrorMessage(DWORD errorCode)
{
	return std::system_category().message(static_cast<int>(errorCode));
#if 0
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;

	FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
		nullptr, errorCode,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr);
#ifdef UNICODE
	LPCTSTR lpWStr = (LPCTSTR)lpMsgBuf;
	int     bufSize = WideCharToMultiByte(CP_ACP, 0, lpWStr, -1, nullptr, 0, nullptr, nullptr);
	std::string res;
	res.resize(bufSize);
	::WideCharToMultiByte(CP_ACP, 0, lpWStr, -1, res.data(), res.size(), nullptr, nullptr);
	::LocalFree(lpMsgBuf);
	return res;
#else
	std::string res((LPTSTR)lpMsgBuf);
	::LocalFree(lpMsgBuf);
	return res;
#endif
#endif
}

std::string wm_sensors::windowsLastErrorMessage()
{
	return windowsErrorMessage(::GetLastError());
}
