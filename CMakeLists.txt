project (raycast LANGUAGES CXX)
cmake_minimum_required (VERSION 3.17)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -march=native")
set (CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -O0 -g")
set (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Ofast")

# Set the default build type if none was specified
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set (CMAKE_BUILD_TYPE "Release")
endif()

find_package (SDL2 REQUIRED)
include_directories (${SDL2_INCLUDE_DIRS})

add_executable (raycast main.cpp)
target_link_libraries (raycast ${SDL2_LIBRARIES})
