cmake_minimum_required(VERSION 3.14)

project(VREnvironment)

add_subdirectory(${CGV_DIR} ${CMAKE_BINARY_DIR}/cgv)
add_subdirectory(vendor/glm)

add_subdirectory(opengl)
add_subdirectory(image)

add_library(vr_env
        vr_env.cpp

        simple_renderer.cpp
        deferred_renderer.cpp

        landscape/Branch.cpp
        landscape/Clouds.cpp
        landscape/Landscape.cpp
        landscape/Terrain.cpp
        landscape/TerrainParams.cpp
        landscape/Trees.cpp)
set_target_properties(
        vr_env PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

target_link_libraries(vr_env PRIVATE
        opengl
        image
        cgv_gl)
add_dependencies(vr_env cgv_viewer cg_fltk crg_grid crg_stereo_view)
