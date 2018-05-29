// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#ifdef _WIN32
// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#include <windows.h>
#endif

#include "Shared/Shared.hpp"

#include <memory>
using std::shared_ptr;
using std::make_shared;
using std::unique_ptr;
using std::make_unique;

#include <string>
using std::string;

using std::runtime_error;
