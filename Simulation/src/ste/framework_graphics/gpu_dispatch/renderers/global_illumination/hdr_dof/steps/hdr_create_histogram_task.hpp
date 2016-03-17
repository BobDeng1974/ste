// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.hpp"
#include "hdr_dof_postprocess.hpp"

#include "gl_current_context.hpp"
#include "hdr_compute_minmax_task.hpp"

namespace StE {
namespace Graphics {

class hdr_create_histogram_task : public gpu_task {
	using Base = gpu_task;
	
private:
	hdr_dof_postprocess *p;

public:
	hdr_create_histogram_task(hdr_dof_postprocess *p) : p(p) {
		gpu_task::sub_tasks.insert(std::make_shared<hdr_compute_minmax_task>(p));
	}
	~hdr_create_histogram_task() noexcept {}

protected:
	void set_context_state() const override final;
	void dispatch() const override final;
};

}
}
