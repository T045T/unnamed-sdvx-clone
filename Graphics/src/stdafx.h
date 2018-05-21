/* Precompiled header file for graphics library */
#pragma once

// OpenGL headers
#include "GL.hpp"

#ifdef _WIN32
// Windows Header File
#include <windows.h>
#endif

#include <SDL2/SDL.h>
#include <Shared/Shared.hpp>

// Asset loading macro
#define CheckedLoad(__stmt) if(!(__stmt)){Logf("Failed to load asset [%s]", Logger::Error, #__stmt); return false; }