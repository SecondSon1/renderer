cmake_minimum_required(VERSION 3.28)

project(Renderer LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory("external/eigen")
add_subdirectory("external/sdl")
add_subdirectory("external/glog")

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/external/imgui")
include_directories("${PROJECT_SOURCE_DIR}/external/stb")

add_executable(main "src/main.cpp"
    "src/graphics/sdl_wrap.cpp"
    "src/scene/mesh.cpp"
    "src/app.cpp"
    "src/util/timer.cpp"
    "src/scene/camera.cpp"
    "src/view.cpp"
    "src/fs/datalist_parser.cpp"
    "src/fs/obj_parser.cpp"
	"src/fs/loader.cpp"
    "src/linalg.cpp"
    "src/renderer.cpp"
    "src/except.cpp"
    "src/fs/root_finder.cpp"
    "src/scene/scene.cpp"
	"src/util/timings_measurer.cpp"
    "src/scenes/example_scene.cpp"
    "src/util/drawing_primitives.cpp"
    "src/controller.cpp"
    "src/util/util.cpp"
    "src/scene/lighting.cpp"
	"src/fs/texture_reader.cpp"

    "external/imgui/imgui.cpp"
	"external/imgui/imgui_draw.cpp"
	"external/imgui/imgui_demo.cpp"
	"external/imgui/imgui_tables.cpp"
	"external/imgui/imgui_widgets.cpp"
	"external/imgui/backends/imgui_impl_sdl3.cpp"
	"external/imgui/backends/imgui_impl_sdlrenderer3.cpp"
)

target_compile_features(main PRIVATE cxx_std_20)
target_link_libraries(main PRIVATE SDL3::SDL3)
target_link_libraries(main PRIVATE glog::glog)
target_link_libraries(main PRIVATE Eigen3::Eigen)
