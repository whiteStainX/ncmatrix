cmake_minimum_required(VERSION 3.16)
project(ncmatrix LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# --- notcurses via pkg-config ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(NOTCURSES QUIET IMPORTED_TARGET notcurses)
if (NOT NOTCURSES_FOUND)
  pkg_check_modules(NOTCURSES REQUIRED IMPORTED_TARGET notcurses-core)
endif()

# --- sources ---
set(MATRIX_SOURCES
  src/main.cpp
)

add_executable(ncmatrix ${MATRIX_SOURCES} ${ANIMATION_SOURCES})

# Add a custom target to track changes in why.toml
add_custom_target(config_dependency ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/frame.toml)
add_dependencies(ncmatrix config_dependency)

# --- includes ---
target_include_directories(ncmatrix PRIVATE
  src
  external/cxxopts
  external/tomlplusplus
)

# --- link notcurses (and its transitive deps) ---
target_link_libraries(ncmatrix PRIVATE PkgConfig::NOTCURSES)
