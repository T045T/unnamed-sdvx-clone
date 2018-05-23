# Graphics library
message("> Configuring Graphics")

# Find files used for project
file(GLOB Graphics_src "src/*.cpp" "src/glew.c" "include/Graphics/*.hpp")

# Platform specific source files
if(WIN32)
	include_directories("src/Windows")
	file(GLOB Platform_src "src/Windows/*.cpp" "src/Windows/*.hpp")
	# Platform specific source group
	source_group("Source Files\\Windows" FILES ${Platform_src})
else(WIN32)
	include_directories("src/Linux")
	file(GLOB Platform_src "src/Linux/*.cpp")
endif(WIN32)

file(GLOB Graphics_src ${Graphics_src} ${Platform_src})

# Compiler stuff
enable_cpp17()
enable_precompiled_headers("${Graphics_src}" src/stdafx.cpp)
precompiled_header_exclude("src/glew.c")

include_directories(include include/Graphics src include/Graphics/glew)
add_library(Graphics ${Graphics_src})

# Public include paths for library
target_include_directories(Graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/Graphics/glew)
	
# GLEW is included statically and also doesn't need GLU(Which doesn't even exist on linux)
add_definitions(-DGLEW_NO_GLU -DGLEW_STATIC)

# Dependencies
target_link_libraries(Graphics PUBLIC Shared)
target_link_libraries(Graphics PUBLIC SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(Graphics PUBLIC ZLIB::ZLIB)
target_link_libraries(Graphics PUBLIC Freetype::Freetype)
target_link_libraries(Graphics PUBLIC ${JPEG_LIBRARIES})
target_link_libraries(Graphics PUBLIC PNG::PNG)
target_include_directories(Graphics PUBLIC ${JPEG_INCLUDE_DIR})