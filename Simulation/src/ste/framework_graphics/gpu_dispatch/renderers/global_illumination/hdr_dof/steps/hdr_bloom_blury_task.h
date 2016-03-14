// StE
// � Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"
#include "hdr_dof_postprocess.h"

#include "gl_current_context.h"
#include "hdr_bloom_blurx_task.h"

namespace StE {
namespace Graphics {

class hdr_bloom_blury_task : public gpu_task {
private:
	hdr_dof_postprocess *p;

public:
	hdr_bloom_blury_task(hdr_dof_postprocess *p) : p(p) {
		gpu_task::add_dependency(std::make_shared<hdr_bloom_blurx_task>(p));
	}
	~hdr_bloom_blury_task() noexcept {}

	void set_context_state() const override final {
		ScreenFillingQuad.vao()->bind();
		p->fbo_hdr_final.bind();
		p->hdr_bloom_blury->bind();
	}
	
	void dispatch() const override final {
		gl_current_context::get()->draw_arrays(GL_TRIANGLE_STRIP, 0, 4);
	}
};

}
}
