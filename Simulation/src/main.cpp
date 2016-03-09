
#include "stdafx.h"

#include "gl_utils.h"
#include "Log.h"
#include "Keyboard.h"
#include "Pointer.h"
#include "StEngineControl.h"
#include "GIRenderer.h"
#include "SphericalLight.h"
#include "DirectionalLight.h"
#include "BasicRenderer.h"
#include "MeshRenderable.h"
#include "CustomRenderable.h"
#include "ModelFactory.h"
#include "Camera.h"
#include "GLSLProgram.h"
#include "GLSLProgramFactory.h"
#include "SurfaceFactory.h"
#include "Texture2D.h"
#include "Scene.h"
#include "Object.h"
#include "ObjectGroup.h"
#include "TextManager.h"
#include "AttributedString.h"
#include "RGB.h"
#include "Sphere.h"
#include "renderable.h"

using namespace StE::LLR;
using namespace StE::Text;

class SkyDome : public StE::Graphics::MeshRenderable<StE::Graphics::mesh_subdivion_mode::Triangles> {
private:
	using ProjectionSignalConnectionType = StE::StEngineControl::projection_change_signal_type::connection_type;

	std::unique_ptr<Texture2D> stars_tex;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

public:
	SkyDome(const StE::StEngineControl &ctx) : MeshRenderable(ctx.glslprograms_pool().fetch_program_task({ "transform_sky.vert", "frag_sky.frag" })(),
															  std::make_shared<StE::Graphics::Sphere>(10, 10, .0f)) {
		stars_tex = StE::Resource::SurfaceFactory::load_texture_2d_task("Data/textures/stars.jpg", true)();

		get_program()->set_uniform("sky_luminance", 5.f);
		get_program()->set_uniform("projection", ctx.projection_matrix());
		get_program()->set_uniform("near", ctx.get_near_clip());
		get_program()->set_uniform("far", ctx.get_far_clip());
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &proj, float, float clip_near, float clip_far) {
			this->get_program()->set_uniform("projection", proj);
			this->get_program()->set_uniform("near", clip_near);
			this->get_program()->set_uniform("far", clip_far);
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	void set_model_matrix(const glm::mat4 &m) {
		get_program()->set_uniform("view_model", m);
	}

	virtual void prepare() const override {
		MeshRenderable::prepare();

		0_tex_unit = *stars_tex;
	}
};

class RayTracer : public StE::Graphics::renderable {
private:
	using ProjectionSignalConnectionType = StE::StEngineControl::projection_change_signal_type::connection_type;
	std::shared_ptr<ProjectionSignalConnectionType> projection_change_connection;

public:
	RayTracer(const StE::StEngineControl &ctx) : StE::Graphics::renderable(ctx.glslprograms_pool().fetch_program_task({ "passthrough.vert", "ray.frag" })()) {
		get_program()->set_uniform("inv_projection", glm::inverse(ctx.projection_matrix()));
		projection_change_connection = std::make_shared<ProjectionSignalConnectionType>([=](const glm::mat4 &proj, float, float clip_near, float clip_far) {
			get_program()->set_uniform("inv_projection", glm::inverse(proj));
		});
		ctx.signal_projection_change().connect(projection_change_connection);
	}

	void set_model_matrix(const glm::mat4 &m) {
		get_program()->set_uniform("inv_view_model", glm::inverse(m));
	}

	virtual void prepare() const override {
		renderable::prepare();

		StE::Graphics::ScreenFillingQuad.vao()->bind();
	}

	virtual void render() const override {
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
};

int main() {
	StE::Log logger("Global Illumination");
//	logger.redirect_std_outputs();
	ste_log_set_global_logger(&logger);
	ste_log() << "Simulation is running";

	int w = 1688, h = 950;
	constexpr float clip_far = 3000.f;
	constexpr float clip_near = 5.f;

	gl_context::context_settings settings;
	settings.vsync = false;
	// settings.fs = true;
	StE::StEngineControl ctx(std::make_unique<gl_context>(settings, "Shlomi Steinberg - Global Illumination", glm::i32vec2{ w, h }));// , gli::FORMAT_RGBA8_UNORM));
	ctx.set_clipping_planes(clip_near, clip_far);

	using ResizeSignalConnectionType = StE::StEngineControl::framebuffer_resize_signal_type::connection_type;
	std::shared_ptr<ResizeSignalConnectionType> resize_connection;
	resize_connection = std::make_shared<ResizeSignalConnectionType>([&](const glm::i32vec2 &size) {
		w = size.x;
		h = size.y;
	});
	ctx.signal_framebuffer_resize().connect(resize_connection);

	SkyDome skydome(ctx);
	StE::Graphics::Scene scene(ctx);
	std::shared_ptr<StE::Graphics::ObjectGroup> object_group = std::make_shared<StE::Graphics::ObjectGroup>(&scene.scene_properties());
	StE::Graphics::GIRenderer renderer(ctx, &scene);
	StE::Graphics::BasicRenderer basic_renderer;
	
	scene.add_object(object_group);

	StE::Graphics::Camera camera;
	camera.set_position({ 25.8, 549.07, -249.2 });
	camera.lookat({ 26.4, 548.5, 248.71 });

	const glm::vec3 light_pos({ -700.6, 138, -70 });
	auto light0 = std::make_shared<StE::Graphics::SphericalLight>(2000.f, StE::Graphics::RGB({ 1.f, .57f, .16f }), light_pos, 10.f);
	auto light1 = std::make_shared<StE::Graphics::DirectionalLight>(1.f, StE::Graphics::RGB({ 1.f, 1.f, 1.f }), glm::normalize(glm::vec3(0.1f, -2.5f, 0.1f)));
	scene.scene_properties().lights_storage().add_light(light0);
	scene.scene_properties().lights_storage().add_light(light1);
	StE::Text::TextManager text_renderer(ctx, StE::Text::Font("Data/ArchitectsDaughter.ttf"));

	StE::Graphics::CustomRenderable fb_clearer{ [&]() { ctx.gl()->clear_framebuffer(); } };
	StE::Graphics::CustomRenderable fb_depth_clearer{ [&]() { ctx.gl()->clear_framebuffer(false); } };


	bool running = true;

	// Bind input
	auto keyboard_listner = std::make_shared<decltype(ctx)::hid_keyboard_signal_type::connection_type>(
		[&](StE::HID::keyboard::K key, int scanline, StE::HID::Status status, StE::HID::ModifierBits mods) {
		using namespace StE::HID;
		auto time_delta = ctx.time_per_frame().count();

		if (status != Status::KeyDown)
			return;

		if (key == keyboard::K::KeyESCAPE)
			running = false;
		if (key == keyboard::K::KeyPRINT_SCREEN || key == keyboard::K::KeyF12)
			ctx.capture_screenshot();
	});
	ctx.hid_signal_keyboard().connect(keyboard_listner);
	ctx.set_pointer_hidden(true);


	bool loaded = false;
	auto model_future = ctx.scheduler().schedule_now(StE::Resource::ModelFactory::load_model_task(ctx, 
																								  R"(Data/models/crytek-sponza/sponza.obj)",  
																								  &*object_group, 
																								  &scene.scene_properties(),
																								  2.5f));

	std::shared_ptr<StE::Graphics::Object> light_obj;
	{
		std::unique_ptr<StE::Graphics::mesh<StE::Graphics::mesh_subdivion_mode::Triangles>> m = std::make_unique<StE::Graphics::mesh<StE::Graphics::mesh_subdivion_mode::Triangles>>();
		std::vector<StE::Graphics::ObjectVertexData> vertices;
		StE::Graphics::ObjectVertexData v;
		v.p = { -100,-10,0 };
		v.uv = { 0,0 };
		vertices.push_back(v);
		v.p = { 100,-10,0 };
		v.uv = { 1,0 };
		vertices.push_back(v);
		v.p = { -50,100,0 };
		v.uv = { 0,1 };
		vertices.push_back(v);
		m->set_vertices(vertices);
		m->set_indices(std::vector<std::uint32_t>{0,1,2});
		light_obj = std::make_shared<StE::Graphics::Object>(std::move(m));

		std::unique_ptr<StE::Graphics::Sphere> sphere = std::make_unique<StE::Graphics::Sphere>(10, 10);
		light_obj = std::make_shared<StE::Graphics::Object>(std::move(sphere));

		light_obj->set_model_matrix(glm::scale(glm::translate(glm::mat4(), light_pos), glm::vec3(10, 10, 10)));

		gli::texture2d light_color_tex{ gli::format::FORMAT_RGB8_UNORM_PACK8, {1,1}, 1 };
		glm::vec3 c = light0->get_diffuse();
		*reinterpret_cast<glm::u8vec3*>(light_color_tex.data()) = glm::u8vec3(c.r * 255.5f, c.g * 255.5f, c.b * 255.5f);

		auto light_mat = std::make_shared<StE::Graphics::Material>();
		light_mat->set_diffuse(std::make_shared<StE::LLR::Texture2D>(light_color_tex, false));
		light_mat->set_emission(c * light0->get_luminance());

		light_obj->set_material_id(scene.scene_properties().material_storage().add_material(light_mat));

		object_group->add_entity(light_obj);
	}


	ctx.set_renderer(&basic_renderer);

	while (!loaded && running) {
		ctx.renderer()->queue().push_back(&fb_clearer);

		{
			using namespace StE::Text::Attributes;
			AttributedWString str = center(stroke(blue_violet, 2)(purple(vvlarge(b(L"Global Illumination\n")))) +
										   azure(large(L"Loading...\n")) +
										   orange(regular(L"By Shlomi Steinberg")));
			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

			ctx.renderer()->queue().push_back(text_renderer.render({ w / 2, h / 2 + 100 }, str));
			ctx.renderer()->queue().push_back(text_renderer.render({ 10, 20 }, vsmall(b(L"Thread pool workers: ") +
																					  olive(std::to_wstring(ctx.scheduler().get_sleeping_workers())) + 
																			 		  L"/" + 
																					  olive(std::to_wstring(ctx.scheduler().get_workers_count())))));
			ctx.renderer()->queue().push_back(text_renderer.render({ 10, 50 },
																   vsmall(b(blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB"))));
		}

		if (model_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			loaded = true;

		ctx.run_loop();
	}


	ctx.set_renderer(&renderer);

	float time = 0;
	while (running) {
		if (ctx.window_active()) {
			auto time_delta = ctx.time_per_frame().count();

			using namespace StE::HID;
			constexpr float movement_factor = 155.f;
			if (ctx.get_key_status(keyboard::K::KeyW) == Status::KeyDown)
				camera.step_forward(time_delta*movement_factor);
			if (ctx.get_key_status(keyboard::K::KeyS) == Status::KeyDown)
				camera.step_backward(time_delta*movement_factor);
			if (ctx.get_key_status(keyboard::K::KeyA) == Status::KeyDown)
				camera.step_left(time_delta*movement_factor);
			if (ctx.get_key_status(keyboard::K::KeyD) == Status::KeyDown)
				camera.step_right(time_delta*movement_factor);

			constexpr float rotation_factor = .09f;
			auto pp = ctx.get_pointer_position();
			auto center = static_cast<glm::vec2>(ctx.get_backbuffer_size())*.5f;
			ctx.set_pointer_position(static_cast<glm::ivec2>(center));
			auto diff_v = (center - static_cast<decltype(center)>(pp)) * time_delta * rotation_factor;
			camera.pitch_and_yaw(-diff_v.y, diff_v.x); 
		}


		auto mv = camera.view_matrix();
		auto mvnt = camera.view_matrix_no_translation();

		float angle = time * glm::pi<float>() / 2.5f;
		glm::vec3 lp = light_pos + glm::vec3(glm::sin(angle) * 3, 0, glm::cos(angle)) * 135.f;

		light0->set_position(lp);

		light_obj->set_model_matrix(glm::scale(glm::translate(glm::mat4(), lp), glm::vec3(light0->get_radius() / 2.f)));
		renderer.update_model_matrix_from_camera(camera);
		skydome.set_model_matrix(mvnt);

		renderer.queue().push_back(&fb_depth_clearer);
		renderer.queue().push_back(&scene);
		renderer.queue().push_back(&skydome);

		{
			using namespace StE::Text::Attributes;

			static float tpf = .0f;
			static unsigned tpf_count = 0;
			static float total_tpf = .0f;
			total_tpf += ctx.time_per_frame().count();
			++tpf_count;
			if (tpf_count % 10 == 0) {
				tpf = total_tpf / 10.f;
				total_tpf = .0f;
			}
			auto tpf_str = std::to_wstring(tpf);

			auto total_vram = std::to_wstring(ctx.gl()->meminfo_total_available_vram() / 1024);
			auto free_vram = std::to_wstring(ctx.gl()->meminfo_free_vram() / 1024);

			renderer.postprocess_queue().push_back(text_renderer.render({ 30, h - 50 },
																		vsmall(b(stroke(dark_magenta, 1)(red(tpf_str)))) + L" ms"));
			renderer.postprocess_queue().push_back(text_renderer.render({ 30, 20 },
																		vsmall(b((blue_violet(free_vram) + L" / " + stroke(red, 1)(dark_red(total_vram)) + L" MB")))));
		}

		time += ctx.time_per_frame().count();
		if (!ctx.run_loop()) break;
	}
	 
	return 0;
}
