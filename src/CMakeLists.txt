add_executable(block
  main.cpp
  application.cpp
  assets.cpp
  cursor_layer.cpp
  gameplay_scene.cpp
  grid.cpp
  imgui_layer.cpp
  log.cpp
  main_menu_scene.cpp
  player.cpp
  scene_host_layer.cpp
  scene_manager.cpp
  window_state.cpp
)

target_link_libraries(block PRIVATE
  raylib_hpp_modules
  raygui_hpp_modules
  imgui
  entt
  spdlog_headers
)

# Application/ImGuiLayer use raylib's bundled GLFW (glfwGetCurrentContext)
# and raylib's C headers directly; main.cpp goes through `import raylib;`
# instead. Both paths coexist in distinct TUs.
target_include_directories(block PRIVATE
  ${CMAKE_SOURCE_DIR}/deps/Raylib-Hpp/refs/raylib/src
  ${CMAKE_SOURCE_DIR}/deps/Raylib-Hpp/refs/raylib/src/external/glfw/include
)

target_compile_features(block PUBLIC cxx_std_23)

set_target_properties(block PROPERTIES OUTPUT_NAME "block")
