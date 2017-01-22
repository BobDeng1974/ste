// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include "stdafx.hpp"

#include "gpu_task_dispatch_queue.hpp"

#include <string>

namespace StE {

class ste_engine_control;

namespace Graphics {

class rendering_system {
protected:
	gpu_task_dispatch_queue q;

public:
	virtual ~rendering_system() noexcept {}

	virtual void render_queue() = 0;

	virtual std::string rendering_system_name() const = 0;
};

}
}
