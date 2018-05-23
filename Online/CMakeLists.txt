# Online library
message("> Configuring Online")

# Find files used for project
file(GLOB Online_src "src/*.cpp" "include/Online/*.hpp")

# Compiler stuff
enable_cpp17()
enable_precompiled_headers("${Online_src}" src/stdafx.cpp)

add_library(Online ${Online_src})

# Public include paths for library
target_include_directories(Online PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Dependencies
target_link_libraries(Online PRIVATE Shared)
target_link_libraries(Online PUBLIC ${CURL_LIBRARIES})
target_link_libraries(Online PUBLIC ${DISCORD_RPC_LIBRARIES})
target_include_directories(Online PUBLIC ${CURL_INCLUDE_DIRS})
target_include_directories(Online PUBLIC ${RAPIDJSON_INCLUDE_DIRS})