#include "vr_env.h"

#include <cgv_gl/box_renderer.h>

vr_env::vr_env() { set_name("vr_env"); }

void vr_env::stream_help(std::ostream &os) {
	os << "Generates a default background for 3D visualizations in virtual reality" << std::endl;
}

bool vr_env::init(cgv::render::context &ctx) {
	auto &deferred = ref_deferred_renderer(ctx, 1);
	if (!deferred.init(ctx)) {
		return false;
	}

	auto &flat_color = ref_flat_color_renderer(ctx, 1);
	if (!flat_color.init(ctx)) {
		return false;
	}

	auto &clouds_renderer = ref_clouds_renderer(ctx, 1);
	if (!clouds_renderer.init(ctx)) {
		return false;
	}

	auto &terrain_renderer = ref_terrain_renderer(ctx, 1);
	if (!terrain_renderer.init(ctx)) {
		return false;
	}

	test_texture = cgv::render::texture();
	if (!test_texture.create_from_image(ctx, "../../texture_test.bmp")) {
		std::cerr << "failed to create test texture: " << test_texture.last_error << std::endl;
		return false;
	}

	if (!trees.init(ctx)) {
		return false;
	}

	return true;
}

void vr_env::clear(cgv::render::context &ctx) {
	ref_flat_color_renderer(ctx, -1);
	ref_deferred_renderer(ctx, -1);
	ref_clouds_renderer(ctx, -1);
	ref_terrain_renderer(ctx, -1);
	trees.clear(ctx);
}

void vr_env::init_frame(cgv::render::context &ctx) { drawable::init_frame(ctx); }

void vr_env::draw(cgv::render::context &ctx) {
	auto enabled_lights = ctx.get_nr_enabled_light_sources();
	//	for (int i = 0; i < enabled_lights; i++) {
	//		auto handle = ctx.get_enabled_light_source_handle(i);
	//		ctx.remove_light_source(handle);
	//	}
	//
	//	auto light = cgv::media::illum::light_source();
	//	light.set_type(cgv::media::illum::LT_DIRECTIONAL);
	//	light.set_position(vec3(0, 1, 0));
	//	light.set_emission(rgb(1, 1, 1));
	//	light.set_ambient_scale(0.1);
	//	ctx.add_light_source(light);

	enabled_lights = ctx.get_nr_enabled_light_sources();
	for (int i = 0; i < enabled_lights; i++) {
		auto handle = ctx.get_enabled_light_source_handle(i);
		auto &l = ctx.get_light_source(handle);
		std::cout << "Light " << i << std::endl;
		std::cout << "  Type:          " << l.get_type() << std::endl;
		std::cout << "  Position:      " << l.get_position() << std::endl;
		std::cout << "  Emission:      " << l.get_emission() << std::endl;
		std::cout << "  Ambient Scale: " << l.get_ambient_scale() << std::endl;
		std::cout << std::endl;
	}

	auto &deferred = ref_deferred_renderer(ctx);
	deferred.set_render_style(deferred_style);
	deferred.render(ctx, [&]() {
		auto &terrain = ref_terrain_renderer(ctx);
		terrain.set_render_style(terrain_style);
		terrain.render(ctx, terrainParams);

		trees.render(ctx, shaderToggles, terrainParams);

		auto &clouds = ref_clouds_renderer(ctx);
		clouds.set_render_style(clouds_style);
		clouds.render(ctx);

		//		auto &flat_color = ref_flat_color_renderer(ctx);
		//		flat_color.set_render_style(flat_color_style);
		//		flat_color.set_position_array(ctx, positions);
		//		flat_color.set_texcoord_array(ctx, texcoords);
		//		flat_color.set_texture(ctx, trees.get_tree_placement_texture());
		//		flat_color.set_indices(ctx, indices);
		//		flat_color.render(ctx, 0, indices.size());
	});
}

void vr_env::finish_draw(cgv::render::context &ctx) { drawable::finish_draw(ctx); }

void vr_env::create_gui() {
	add_decorator("VR Environment", "heading", "level=2");

	if (begin_tree_node("Deferred Rendering", deferred_style)) {
		align("\a");
		add_gui("deferred style", deferred_style);
		align("\b");
		end_tree_node(deferred_style);
	}

	if (begin_tree_node("flat color style", flat_color_style)) {
		align("\a");
		add_gui("flat color style", flat_color_style);
		align("\b");
		end_tree_node(flat_color_style);
	}

	if (begin_tree_node("Terrain", terrain_style)) {
		align("\a");
		add_gui("terrain style", terrain_style);
		align("\b");
		end_tree_node(terrain_style);
	}

	if (begin_tree_node("Clouds", clouds_style)) {
		align("\a");
		add_gui("clouds", clouds_style);
		align("\b");
		end_tree_node(clouds_style);
	}

	if (begin_tree_node("Trees", trees)) {
		align("\a");
		add_gui("Trees", trees);
		align("\b");
		end_tree_node(trees);
	}

	add_member_control(this, "Seed", terrainParams.seed);
	connect_copy(add_button("New Seed")->click, cgv::signal::rebind(this, &vr_env::generate_new_seed));
	add_member_control(this, "Steepness", terrainParams.power);
	add_member_control(this, "Bowl Strength", terrainParams.bowlStrength);
	add_member_control(this, "Platform Height", terrainParams.platformHeight);

	if (begin_tree_node_void("Noise Layers", this)) {
		align("\a");
		for (int i = 0; i < static_cast<int>(terrainParams.noiseLayers.size()); i++) {
			const auto enabledLabel = "Enabled " + std::to_string(i);
			add_member_control(this, enabledLabel, terrainParams.noiseLayers[i].enabled);

			const auto frequencyLabel = "Frequency " + std::to_string(i);
			add_member_control(this, frequencyLabel, terrainParams.noiseLayers[i].frequency);

			const auto amplitudeLabel = "Amplitude " + std::to_string(i);
			add_member_control(this, amplitudeLabel, terrainParams.noiseLayers[i].amplitude);
		}

		connect_copy(add_button("Add Noise Layer")->click, cgv::signal::rebind(this, &vr_env::add_new_noise_layer));

		align("\a");
		end_tree_node_void(this);
	}
}

bool vr_env::handle(cgv::gui::event &e) { return false; }

std::string vr_env::get_type_name() const { return "vr_env"; }

void vr_env::on_set(void *member_ptr) {
	// NOTE: on_set is called every time something in the gui changes, thus we have to post a redraw event to make
	// sure any changes to the render styles ar being applied
	update_member(member_ptr);
	post_redraw();
}

void vr_env::generate_new_seed() {
	std::random_device rand_dev;
	std::mt19937 generator(rand_dev());
	std::uniform_int_distribution<int> distribution(0, 1000000);
	terrainParams.seed = distribution(generator);
	post_recreate_gui();
	post_redraw();
}

void vr_env::add_new_noise_layer() {
	terrainParams.noiseLayers.emplace_back();
	post_recreate_gui();
	post_redraw();
}

cgv::base::object_registration<vr_env> vr_env_reg("vr_env"); // NOLINT(cert-err58-cpp)
