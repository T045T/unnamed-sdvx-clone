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

if(APPLE)
  target_link_libraries(Main "-L${SDL2_LIBDIR} -lSDL2main")
else(APPLE)
  target_link_libraries(Main SDL2::SDL2main)
endif(APPLE)
