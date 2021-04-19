cmake_minimum_required(VERSION 3.14)

project(VREnvironment)

add_subdirectory(vendor/cgv)
add_subdirectory(vendor/glm)

add_subdirectory(opengl)
add_subdirectory(image)

add_library(vr_env vr_env.cpp simple_renderer.cpp)

# TODO reduce the number of dependencies (some might not be needed)
target_link_libraries(vr_env PRIVATE
        opengl
        image

        cgv_utils
        cgv_type
        cgv_reflect
        cgv_data
        cgv_base
        cgv_math
        cgv_media
        cgv_gui
        cgv_render
        cgv_os
        cgv_signal
        cg_fltk
        crg_grid
        cgv_gl
        crg_stereo_view)
add_dependencies(vr_env cgv_viewer)
