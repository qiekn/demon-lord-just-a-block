add_executable(block
  main.cpp
  imgui_layer.cpp
)

target_link_libraries(block PRIVATE
  raylib_hpp_modules
  imgui
  entt
)

# imgui_layer.cpp talks to raylib's bundled GLFW directly to wire the
# imgui_impl_glfw backend onto the same window raylib creates.
target_include_directories(block PRIVATE
  ${CMAKE_SOURCE_DIR}/deps/Raylib-Hpp/refs/raylib/src/external/glfw/include
)

target_compile_features(block PUBLIC cxx_std_23)

set_target_properties(block PROPERTIES OUTPUT_NAME "block")
