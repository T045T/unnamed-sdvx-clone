/*
	Useful macros
 */
#pragma once

#define _STRINGIFY(__x) #__x
#define STRINGIFY(__x) _STRINGIFY(__x)

#define _EXPAND(__x) __x
#define EXPAND(__x) _EXPAND(__x)

#define CONCAT(__a, __b) _CONCAT(__a, __b)
