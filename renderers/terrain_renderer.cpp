#include "terrain_renderer.h"

#include <cgv_gl/gl/gl_tools.h>

terrain_renderer &ref_terrain_renderer(cgv::render::context &ctx, int ref_count_change) {
	static int ref_count = 0;
	static terrain_renderer r;
	r.manage_singelton(ctx, "terrain_renderer", ref_count, ref_count_change);
	return r;
}

cgv::render::render_style *terrain_renderer::create_render_style() const { return new terrain_render_style(); }

bool terrain_renderer::validate_attributes(const cgv::render::context &ctx) const {
	const auto &strs = get_style<terrain_render_style>();
	return surface_renderer::validate_attributes(ctx);
}

bool terrain_renderer::init(cgv::render::context &ctx) {
	bool res = surface_renderer::init(ctx);
	if (!ref_prog().is_created()) {
		if (!ref_prog().build_program(ctx, "terrain.glpr", true)) {
			std::cerr << "ERROR in terrain_renderer::init() ... could not build program terrain.glpr" << std::endl;
			return false;
		}
	}

	init_positions();

	// TODO initialize textures here

	return res;
}

bool terrain_renderer::enable(cgv::render::context &ctx) {
	const auto &strs = get_style<terrain_render_style>();

	if (!surface_renderer::enable(ctx)) {
		return false;
	}

	if (!ref_prog().is_linked()) {
		return false;
	}

	const auto &style = get_style<terrain_render_style>();
	if (!ref_prog().set_uniform(ctx, "flat_color", style.surface_color)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "has_texture", has_texture)) {
		return false;
	}
	if (!ref_prog().set_uniform(ctx, "texture_sampler", 0)) {
		return false;
	}
	return texture.enable(ctx, 0);
}

bool terrain_renderer::disable(cgv::render::context &ctx) { return surface_renderer::disable(ctx); }

bool terrain_render_style_reflect::self_reflect(cgv::reflect::reflection_handler &rh) {
	return rh.reflect_base(*static_cast<terrain_render_style *>(this));
}

void terrain_renderer::set_texture(cgv::render::context &ctx, const cgv::render::texture &t) {
	has_texture = true;
	texture = t;
}

void terrain_renderer::draw(cgv::render::context &ctx, size_t start, size_t count, bool use_strips, bool use_adjacency,
							uint32_t strip_restart_index) {
	draw_impl(ctx, cgv::render::PT_PATCHES, start, count, use_strips, use_adjacency, strip_restart_index);
}

void terrain_renderer::render(cgv::render::context &ctx, const TerrainParams &terrainParams) {
	set_position_array(ctx, custom_positions);
	set_indices(ctx, custom_indices);
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	renderer::render(ctx, 0, custom_indices.size());
}

void terrain_renderer::init_positions() {
	custom_positions.clear();
	custom_indices.clear();

	std::vector<float> quadVertices = {
		  0.0F, 0.0F, //
		  1.0F, 0.0F, //
		  1.0F, 1.0F, //
		  0.0F, 1.0F, //
	};

	// generate a 10x10 grid of points in the range of -500 to 500
	int width = 10;
	int height = 10;
	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			for (int i = 0; i < static_cast<int64_t>(quadVertices.size()); i++) {
				float f = quadVertices[i];
				if (i % 2 == 0) {
					f += static_cast<float>(row);
				} else {
					f += static_cast<float>(col);
				}
				f -= 5.0F;
				f *= 100.0F;
				custom_positions.push_back(f);
			}
		}
	}

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			const int i = (row * width + col) * 4;
			custom_indices.emplace_back(i);
			custom_indices.emplace_back(i + 1);
			custom_indices.emplace_back(i + 2);
			custom_indices.emplace_back(i);
			custom_indices.emplace_back(i + 2);
			custom_indices.emplace_back(i + 3);
		}
	}
}

cgv::reflect::extern_reflection_traits<terrain_render_style, terrain_render_style_reflect>
get_reflection_traits(const terrain_render_style &) {
	return cgv::reflect::extern_reflection_traits<terrain_render_style, terrain_render_style_reflect>();
}

#include <cgv/gui/provider.h>

struct terrain_render_style_gui_creator : public cgv::gui::gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider *p, const std::string &label, void *value_ptr, const std::string &value_type,
				const std::string &gui_type, const std::string &options, bool *) override {
		if (value_type != cgv::type::info::type_name<terrain_render_style>::get_name())
			return false;
		auto *strs_ptr = reinterpret_cast<terrain_render_style *>(value_ptr);
		p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style *>(strs_ptr));
		return true;
	}
};

cgv::gui::gui_creator_registration<terrain_render_style_gui_creator>
	  terrain_rs_gc_reg("terrain_render_style_gui_creator");
