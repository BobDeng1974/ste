// StE
// � Shlomi Steinberg, 2015

#pragma once

#include <stdio.h>
#ifdef _MSC_VER
#include <direct.h>
#define __get_cwd _getcwd
#else
#include <unistd.h>
#define __get_cwd getcwd
#endif

#include <filesystem>

namespace ste {

std::experimental::filesystem::path inline getcwd() {
	char cwd[FILENAME_MAX];
	__get_cwd(cwd, sizeof(cwd));

	return cwd;
}

}
