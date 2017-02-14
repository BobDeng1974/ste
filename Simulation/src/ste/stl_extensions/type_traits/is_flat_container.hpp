// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#include <vector>
#include <array>
#include <type_traits>
#include <boost_flatset.hpp>
#include <boost_flatmap.hpp>

namespace StE {

template <typename V>
struct is_flat_container : public std::false_type {};

template <typename ... Args>
struct is_flat_container<std::vector<Args...>> : public std::true_type {};
template <typename ... Args>
struct is_flat_container<std::array<Args...>> : public std::true_type {};
template <typename ... Args>
struct is_flat_container<boost::container::flat_set<Args...>> : public std::true_type {};
template <typename ... Args>
struct is_flat_container<boost::container::flat_map<Args...>> : public std::true_type {};

}
