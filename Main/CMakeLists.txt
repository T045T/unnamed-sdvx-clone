# Main Project
message("> Configuring Main")

# Find files used for project
file(GLOB Audio_include "include/Audio/*.hpp")
file(GLOB Audio_src "src/Audio/*.cpp")
file(GLOB GUI_include "include/GUI/*.hpp")
file(GLOB GUI_src "src/GUI/*.cpp" "Audio/minimp3.c")
file(GLOB Main_src "src/*.cpp" "include/*.hpp"  "src/Resource.rc" 
	${Audio_include} ${Audio_src} ${GUI_src} ${GUI_include})

# Project filters
source_group("Header Files\\Audio" FILES ${Audio_include})
source_group("Source Files\\Audio" FILES ${Audio_src})
source_group("Header Files\\GUI" FILES ${GUI_include})
source_group("Source Files\\GUI" FILES ${GUI_src})

# Compiler stuff
enable_cpp17()
enable_precompiled_headers("${Main_src}" src/stdafx.cpp)
precompiled_header_exclude("src/Resource.rc")

include_directories(include/Audio include/GUI include)
add_executable(Main ${Main_src} ${C_src})
set_output_postfixes(Main)

find_package(sdl2 REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(Freetype REQUIRED)
#find_package(discord-rpc REQUIRED)

# Target subsystem on windows, set debugging folder
if(MSVC)
   set_target_properties(Main PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
   set_target_properties(Main PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
endif(MSVC)
	
# Dependencies
target_link_libraries(Main Shared)
target_link_libraries(Main Graphics)
target_link_libraries(Main Audio)
target_link_libraries(Main GUI)
target_link_libraries(Main Beatmap)
target_link_libraries(Main Online)

find_path(VORBIS_INCLUDE_DIR vorbis/vorbisfile.h)
find_library(VORBIS_LIBRARY vorbis)
target_include_directories(Main PUBLIC ${VORBIS_INCLUDE_DIR})
target_link_libraries(Main ${VORBIS_LIBRARY})

find_path(OGG_INCLUDE_DIR ogg/ogg.h)
find_library(OGG_LIBRARY ogg)
target_include_directories(Main PUBLIC ${OGG_INCLUDE_DIR})
target_link_libraries(Main ${OGG_LIBRARY})

#target_link_libraries(Main PRIVATE SDL2::SDL2 SDL2::SDL2main)
#target_link_libraries(Main PRIVATE ZLIB::ZLIB)
#target_link_libraries(Main PRIVATE Freetype::Freetype)
#target_link_libraries(Main PRIVATE ${CURL_LIBRARIES})
#target_link_libraries(Main PRIVATE ${JPEG_LIBRARIES})
#target_link_libraries(Main PRIVATE PNG::PNG)
#target_link_libraries(Main discord-rpc)
#target_include_directories(Main PRIVATE ${JPEG_INCLUDE_DIR})
#target_include_directories(Main PRIVATE ${CURL_INCLUDE_DIRS})