#ifndef FMTOUT_H
#define FMTOUT_H

#include <Windows.h>
#include <mutex>

constexpr auto FMTOUT_DEBUG = 1;

inline std::mutex& fmt_mutex() {
	static std::mutex m;
	return m;
}

template <class T>
void _out(const T& out) {
	std::cout << out << ' ';
}

template <typename First, typename ... Rest>
void _out(const First& first, const Rest& ... rest) {
	_out(first);
	_out(rest...);
}

template <typename ... Args>
void fmtout(Args ... args) {
	std::lock_guard<std::mutex> lock(fmt_mutex());

	static const HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hstdout, FOREGROUND_BLUE | FOREGROUND_GREEN);

	SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);

	_out(args...);
	std::cout << '\n';
}

template<typename ... Args>
constexpr void dbgout(Args ... args) {
	if constexpr (FMTOUT_DEBUG) {
		fmtout(args...);
	}
}

#endif