#pragma once

#include <iostream>
#include <string>

class Logger
{
public:
	template <typename... TArgs> inline static void info(TArgs... messageParts)
	{
		info_recurse(messageParts...);
		std::cout << std::endl;
	}

private:
	template <typename T, typename... TArgs>
	inline static void info_recurse(T next, TArgs... rest)
	{
		std::cout << next;
		info_recurse(rest...);
	}
	inline static void info_recurse() {}

public:
	template <typename... TArgs> inline static void warn(TArgs... messageParts)
	{
		warn_recurse(messageParts...);
		std::cout << std::endl;
	}

private:
	template <typename T, typename... TArgs>
	inline static void warn_recurse(T next, TArgs... rest)
	{
		std::cout << next;
		warn_recurse(rest...);
	}
	inline static void warn_recurse() {}

// For platform independant DEBUG conditional:
// https://stackoverflow.com/questions/8591762/ifdef-debug-with-cmake-independent-from-platform
#ifndef NDEBUG
public:
	template <typename... TArgs> inline static void debug(TArgs... messageParts)
	{
		debug_recurse(messageParts...);
		std::cout << std::endl;
	}

private:
	template <typename T, typename... TArgs>
	inline static void debug_recurse(T next, TArgs... rest)
	{
		std::cout << next;
		debug_recurse(rest...);
	}
	inline static void debug_recurse() {}
#else
public:
	template <typename... TArgs> inline static void debug(TArgs...) {}
#endif

public:
	template <typename... TArgs> inline static void error(TArgs... messageParts)
	{
		error_recurse(messageParts...);
		std::cerr << std::endl;
	}

private:
	template <typename T, typename... TArgs>
	inline static void error_recurse(T next, TArgs... rest)
	{
		std::cerr << next;
		error_recurse(rest...);
	}
	inline static void error_recurse() {}
};