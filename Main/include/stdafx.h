/* Main and precompiled header file for Main project*/
#pragma once

// OpenGL headers
#include <Graphics/GL.hpp>

#ifdef _WIN32
// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#endif

#include <Shared/Shared.hpp>

// Graphics components
#include <Graphics/OpenGL.hpp>
#include <Graphics/Image.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/Material.hpp>
#include <Graphics/Mesh.hpp>
#include <Graphics/RenderQueue.hpp>
#include <Graphics/RenderState.hpp>
#include <Graphics/ParticleSystem.hpp>
#include <Graphics/MeshGenerators.hpp>
#include <Graphics/Font.hpp>
#include <Graphics/Framebuffer.hpp>
using namespace Graphics;

// Asset loading macro
#define CheckedLoad(__stmt)\
	if(!(__stmt))\
	{\
		Logf("Failed to load asset [%s]", Logger::Error, #__stmt);\
		throw runtime_error("Failed to load asset");\
	}