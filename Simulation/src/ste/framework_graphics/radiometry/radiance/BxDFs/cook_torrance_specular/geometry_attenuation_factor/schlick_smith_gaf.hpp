// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <stdafx.hpp>

namespace ste {
namespace graphics {

template <typename T>
class schlick_smith_gaf {
private:
	T g1(const glm::tvec3<T> &u, const glm::tvec3<T> &n, const T &k) const {
		auto d = glm::dot(n,u);
		return d / (d * (static_cast<T>(1) - k) + k);
	}

public:
	T operator()(const glm::tvec3<T> &l, const glm::tvec3<T> &v, const glm::tvec3<T> &n, const glm::tvec3<T> &h, const T &roughness) const {
		auto k = (roughness + 1) * (roughness + 1) / static_cast<T>(8);

		return g1(l, n, k) * g1(v, n, k);
	}
};

}
}
