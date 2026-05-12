cmake_minimum_required(VERSION 3.30)

# Toolchain must be set BEFORE project() so the import-std machinery sees it.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

include(${CMAKE_SOURCE_DIR}/cmake/EnableCxxImportStd.cmake)

project(block LANGUAGES C CXX)

set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------: submodule sanity
foreach(_dep IN ITEMS
    "deps/Raylib-Hpp/CMakeLists.txt"
    "deps/imgui/imgui.h"
    "deps/entt/src/entt/entt.hpp")
  if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dep}")
    message(FATAL_ERROR
      "Submodule file missing: ${_dep}. "
      "Run: git submodule update --init --recursive")
  endif()
endforeach()

# Raylib-Hpp expects raylib vendored at deps/Raylib-Hpp/refs/raylib (gitignored
# by the wrapper repo). Clone it locally; CI scripts should do the same.
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/Raylib-Hpp/refs/raylib/CMakeLists.txt")
  message(FATAL_ERROR
    "raylib is missing under deps/Raylib-Hpp/refs/raylib (gitignored by Raylib-Hpp). "
    "Clone it: git clone --depth 1 https://github.com/raysan5/raylib.git "
    "deps/Raylib-Hpp/refs/raylib")
endif()

add_subdirectory(deps)
add_subdirectory(src)
