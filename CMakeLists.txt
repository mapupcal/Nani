cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE
   AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/vcpkg/scripts/buildsystems/vcpkg.cmake")
  set(CMAKE_TOOLCHAIN_FILE
      "${CMAKE_CURRENT_SOURCE_DIR}/deps/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "Vcpkg toolchain file")
endif()

project(Nani VERSION 0.1.0 LANGUAGES C CXX)

# Enable Visual Studio folder feature for source grouping
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(nani_cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(nani)

option(NANI_BUILD_SNIPPETS "Build dependency usage snippets" ON)

if(NANI_BUILD_SNIPPETS)
  add_subdirectory(deps/snippets/simple_skia)
  add_subdirectory(deps/snippets/simple_libuv)
  add_subdirectory(deps/snippets/simple_quickjs_ng)
  add_subdirectory(deps/snippets/simple_curl)
  add_subdirectory(deps/snippets/simple_glfw3)
endif()
