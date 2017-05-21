// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>
#include <gpu_dispatchable.hpp>

#include <ste_engine_control.hpp>
#include <profiler.hpp>

#include <signal.hpp>
#include <font.hpp>

#include <camera.hpp>

#include <lib/map.hpp>
#include <lib/vector.hpp>
#include <lib/unique_ptr.hpp>
#include <lib/string.hpp>
#include <functional>

namespace ste {
namespace graphics {

class debug_gui : public gpu_dispatchable {
	using Base = gpu_dispatchable;

private:
	using hid_pointer_button_signal_connection_type = ste_engine_control::hid_pointer_button_signal_type::connection_type;
	using hid_scroll_signal_connection_type = ste_engine_control::hid_scroll_signal_type::connection_type;
	using hid_keyboard_signal_connection_type = ste_engine_control::hid_keyboard_signal_type::connection_type;

private:
	const ste_engine_control &ctx;
	profiler *prof;
	const camera *cam;

	mutable lib::map<lib::string, lib::vector<float>> prof_tasks_last_samples;

private:
	lib::shared_ptr<hid_pointer_button_signal_connection_type> hid_pointer_button_signal;
	lib::shared_ptr<hid_scroll_signal_connection_type> hid_scroll_signal;
	lib::shared_ptr<hid_keyboard_signal_connection_type> hid_keyboard_signal;

	lib::vector<std::function<void(const glm::ivec2 &)>> user_guis;

public:
	debug_gui(const ste_engine_control &ctx, profiler *prof, const ste::text::font &default_font, const camera *cam = nullptr);
	~debug_gui() noexcept;

	void add_custom_gui(std::function<void(const glm::ivec2 &)> &&f) { user_guis.emplace_back(std::move(f)); }

	bool is_gui_active() const;

protected:
	void set_context_state() const override final {}
	void dispatch() const override final;
};

}
}
