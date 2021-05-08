cmake_minimum_required(VERSION 3.14)

project(VREnvironment)

set(BUILD_SHARED_LIBS ON)

add_subdirectory(${CGV_DIR} ${CMAKE_BINARY_DIR}/cgv)
add_subdirectory(vendor/glm)

add_subdirectory(opengl)
add_subdirectory(image)

file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")
shader_test("${CGV_DIR}"
        ST_FILES
        ST_INCLUDES
        ST_INSTALL_DIR
        ${SHADERS})

add_library(vr_env
        vr_env.cpp

        renderers/clouds_renderer.cpp
        renderers/deferred_renderer.cpp
        renderers/flat_color_renderer.cpp
        renderers/ssao_renderer.cpp
        renderers/terrain_renderer.cpp
        renderers/tree_renderer.cpp

        Trees.cpp
        Branch.cpp
        TerrainParams.cpp

        ${ST_FILES})
set_target_properties(
        vr_env PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

target_link_libraries(vr_env PRIVATE
        opengl
        image
        cgv_media
        cgv_gl
        cmi_io)
add_dependencies(vr_env cgv_viewer cg_fltk crg_grid crg_stereo_view)
