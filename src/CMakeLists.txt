add_executable(block main.cpp)

target_link_libraries(block PRIVATE
  raylib_hpp_modules
  entt
)

target_compile_features(block PUBLIC cxx_std_23)

# Keep the console open during scaffold work so std::print + raylib's
# TraceLog output land somewhere visible. Flip WIN32_EXECUTABLE on later
# once the game is shipping.
set_target_properties(block PROPERTIES OUTPUT_NAME "block")
