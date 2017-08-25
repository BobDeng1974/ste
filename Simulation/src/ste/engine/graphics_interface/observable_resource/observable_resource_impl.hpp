//	StE
// © Shlomi Steinberg 2015-2017

#pragma once

#include <stdafx.hpp>

#include <observable_resource.hpp>
#include <resource_storage_base.hpp>

namespace ste {
namespace gl {

template <typename Descriptor>
void observable_resource<Descriptor>::notify() {
	assert(is_valid() && "Orphaned resource calling notify()!");
	storage_ptr->notify_resource(this);
}

template <typename Descriptor>
void observable_resource<Descriptor>::dealloc() {
	if (storage_ptr != nullptr)
		storage_ptr->erase_resource(this);
}

}
}
