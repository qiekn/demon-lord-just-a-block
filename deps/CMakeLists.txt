# ---------------------------------------------------------------------------: Raylib-Hpp (bundles raylib via refs/raylib)
add_subdirectory(Raylib-Hpp)

# ---------------------------------------------------------------------------: entt as header-only INTERFACE target
# Not used yet by the window scaffold; wired now so client code can just
# #include <entt/entt.hpp> and link `entt`.
add_library(entt INTERFACE)
target_include_directories(entt INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/entt/src
)
