# VR-Environment

## Starting in CLion

Create a `CMake Application` run configuration. Select `vr_env` as the target and `cgv_viewer` as executable. Add the
following as program arguments:

```
plugin:cg_fltk
plugin:crg_stereo_view
plugin:crg_grid
"type(shader_config):shader_path='../../glgl;../../vendor/cgv/libs/cgv_gl/glsl'"
plugin:vr_env
```

It is possible to use `ninja` as the underlying build tool for faster builds. Simply add the following entry
under `File | Settings | Build, Execution, Deployment | CMake | <CMake configuration to change> | CMake options`

```
-G Ninja
-D CMAKE_CXX_COMPILER="C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29910/bin/Hostx86/x64/cl.exe"
-D CMAKE_C_COMPILER="C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.28.29910/bin/Hostx86/x64/cl.exe"
```

Just setting `-G Ninja` alone might also work, but sometimes ninja picks up the wrong compiler.
