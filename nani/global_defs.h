#pragma once

#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <string>
#include <format>
#include <string_view>
#include <cctype>
#include <utility>
#include <ranges>
#include <set>
#include <optional>
#include <any>


#if defined _DEBUG
#	include<cassert>
#	define NANI_ASSERT(expr) assert(expr)
#	define NANI_MESSAGE(c_msg) printf("[WARNNING][%s][file:%s, line:%d]\n", std::string(c_msg).c_str(), __FILE__, __LINE__)
#else
#	define NANI_ASSERT(expr)
#	define NANI_MESSAGE(c_msg)
#endif

#if !defined(NANI_API)
#if defined(_MSC_VER)
#ifdef NANI_IMPLEMENTATION
#define NANI_API __declspec(dllexport)
#else
#define NANI_API __declspec(dllimport)
#endif
#else
#define NANI_API __attribute__((visibility("default")))
#endif
#else
#define NANI_API
#endif

#if defined(_WIN32) || defined(WIN32)
#define NANI_OS_WIN
#endif
