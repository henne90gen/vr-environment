
cmake:
	mkdir -p build && cd build && cmake -G Ninja ..

build:
	cd build && ninja

run: build
	cd build/bin && ./cgv_viewer plugin:cg_fltk plugin:crg_stereo_view plugin:crg_grid "type(shader_config):shader_path='../../glsl;$CGV_DIR/libs/cgv_gl/glsl'" plugin:vr_env

.PHONY: build
