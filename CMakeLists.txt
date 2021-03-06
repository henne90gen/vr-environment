cmake_minimum_required(VERSION 3.14)

project(VREnvironment)

set(BUILD_SHARED_LIBS ON)

find_package(OpenMP REQUIRED)
find_package(CGV REQUIRED)

add_subdirectory(vendor/glm)

file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")
shader_test("${CGV_DIR}"
        ST_FILES
        ST_INCLUDES
        ST_INSTALL_DIR
        ${SHADERS})

add_library(vr_env
        vr_env.cpp

        renderers/clouds_renderer.cpp
        renderers/blur_renderer.cpp
        renderers/deferred_renderer.cpp
        renderers/flat_color_renderer.cpp
        renderers/ssao_renderer.cpp
        renderers/terrain_renderer.cpp
        renderers/tree_renderer.cpp

        Trees.cpp
        Branch.cpp
        Sphere.cpp
        TerrainParams.cpp
        utils.cpp

        ${ST_FILES}
        ${SHADERS})
set_target_properties(
        vr_env PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

target_link_libraries(vr_env PRIVATE
        OpenMP::OpenMP_CXX
        glm
        cgv_media
        cgv_gl
        cmi_io)
add_dependencies(vr_env cgv_viewer cg_fltk crg_light crg_grid crg_stereo_view crg_vr_view)
