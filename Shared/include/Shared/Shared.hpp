/* Shared library main include file */
#pragma once

// Asset loading macro
#define CheckedLoad(__stmt)\
	if(!(__stmt))\
	{\
		Logf("Failed to load asset [%s]", Logger::Error, #__stmt);\
		throw runtime_error("Failed to load asset");\
	}

// Types
#include "Types.hpp"
#include <string>
using std::string;

// Random Utility classes
#include "Unique.hpp"
#include "Utility.hpp"
#include "Delegate.hpp"
#include "Timer.hpp"

// Reference counting
#include <memory>
using std::shared_ptr;
using std::make_shared;
using std::unique_ptr;
using std::make_unique;

// Errors
using std::runtime_error;

// Filsystem headers
#include "FileSystem.hpp"

// Container classes
#include "String.hpp"
#include "Vector.hpp"
#include "Map.hpp"
#include "Set.hpp"
#include "List.hpp"

// Debugging and logging
#include "Log.hpp"

// Maths
#include "Math.hpp"
#include "VectorMath.hpp"
#include "Transform.hpp"
#include "Color.hpp"
#include "Rect.hpp"
#include "Margin.hpp"
#include "Random.hpp"
