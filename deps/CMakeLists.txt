# ---------------------------------------------------------------------------: Raylib-Hpp (bundles raylib via refs/raylib)
add_subdirectory(Raylib-Hpp)

# ---------------------------------------------------------------------------: entt as header-only INTERFACE target
add_library(entt INTERFACE)
target_include_directories(entt INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/entt/src
)

# ---------------------------------------------------------------------------: imgui static lib (docking branch)
# raylib uses glfw + opengl internally on PLATFORM_DESKTOP, so we wire the
# glfw + opengl3 backends. glfw headers come from raylib's bundled copy.
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
set(RAYLIB_GLFW_INCLUDE
    ${CMAKE_SOURCE_DIR}/deps/Raylib-Hpp/refs/raylib/src/external/glfw/include)

add_library(imgui STATIC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
)
target_include_directories(imgui PRIVATE
  ${RAYLIB_GLFW_INCLUDE}
)

# ---------------------------------------------------------------------------: skip module scan on non-module deps for faster configure
foreach(_dep IN ITEMS imgui entt)
  if(TARGET ${_dep})
    set_target_properties(${_dep} PROPERTIES CXX_SCAN_FOR_MODULES OFF)
  endif()
endforeach()
