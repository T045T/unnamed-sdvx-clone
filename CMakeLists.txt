# Root CMake file
cmake_minimum_required(VERSION 3.8)
project(FX)

# Project configurations
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_DEBUG_POSTFIX _Debug)
set(CMAKE_RELEASE_POSTFIX _Release)

# Set output folders
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/bin )
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/bin )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/lib )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

# All projects use unicode define
#	this is mainly for windows functions either being defined to call A or W prefixed functions
add_definitions(-DUNICODE -D_UNICODE)

# Precompiled header macro
#	src 	= Path to source files
#	pchSrc 	= Path to precompiled header source file
macro(enable_precompiled_headers src pchSrc)
	if(MSVC)
		message("Enabling precompiled header generated from source file ${pchSrc}")
		#message("Files using precompiled headers => ${src}")
		# Set precompiled header usage
		set_source_files_properties(${src} PROPERTIES COMPILE_FLAGS "/Yu")
		# Set precompiled header
		set_source_files_properties(${pchSrc} PROPERTIES COMPILE_FLAGS "/Yc")
	endif(MSVC)
endmacro(enable_precompiled_headers)

# Excludes a file from precompiled header usage
macro(precompiled_header_exclude exclude)
	if(MSVC)
		set_source_files_properties(${exclude} PROPERTIES COMPILE_FLAGS "")
	endif(MSVC)
endmacro(precompiled_header_exclude)

# Function to enable c++17 compilation on linux
macro(enable_cpp17)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)
endmacro(enable_cpp17)

# Set output binary postfixes so that they will be named <project>_<configuration>.exe/dll
macro(set_output_postfixes projectName)
	set_target_properties(${projectName} PROPERTIES 
		OUTPUT_NAME_DEBUG ${projectName}_Debug
		OUTPUT_NAME_RELEASE ${projectName}_Release)
endmacro(set_output_postfixes)

if(APPLE)
  execute_process(
    COMMAND bash -c "set -e && PREFIX=`brew --prefix`; echo $PREFIX/include"
    RESULT_VARIABLE BREW_FOUND
    OUTPUT_VARIABLE BREW_INCLUDE)
  if(NOT ${BREW_FOUND} EQUAL 0 OR "${BREW_INCLUDE}" STREQUAL "" OR "${BREW_INCLUDE}" STREQUAL "/include")
    message(FATAL_ERROR "Homebrew not found!\n"
      "Please install Homebrew and use it to install the dependencies:\n"
      "/usr/bin/ruby -e \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)\"")
  else()
    include_directories(${BREW_INCLUDE})
  endif()
endif(APPLE)

# Find all libraries
find_package(sdl2 REQUIRED)
find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(Freetype REQUIRED)

find_package(CURL REQUIRED)
find_package(RapidJSON REQUIRED)
find_library(VORBIS_LIBRARY vorbis)
find_library(VORBIS_FILE_LIBRARY vorbisfile)
find_library(VORBIS_ENC_LIBRARY vorbisenc)
find_library(OGG_LIBRARY ogg)
find_library(DISCORD_RPC_LIBRARY discord-rpc)

# Sub-Project directories
add_subdirectory(Shared)
add_subdirectory(Graphics)
add_subdirectory(Main)
add_subdirectory(Audio)
add_subdirectory(Beatmap)
add_subdirectory(GUI)
add_subdirectory(Online)

# Unit test projects
#add_subdirectory(Tests)
#add_subdirectory(Tests.Shared)
#add_subdirectory(Tests.Game)
