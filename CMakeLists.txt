cmake_minimum_required(VERSION 3.28)

project(Renderer LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory("external/eigen")
add_subdirectory("external/sdl")
add_subdirectory("external/glog")

include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(main src/main.cpp src/sdl_wrap.cpp src/scene/from_obj.cpp "src/scene/mesh.cpp"  )

target_compile_features(main PRIVATE cxx_std_20)
target_link_libraries(main PRIVATE SDL3::SDL3)
target_link_libraries(main PRIVATE glog::glog)
target_link_libraries(main PRIVATE Eigen3::Eigen)