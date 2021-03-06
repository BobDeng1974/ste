// StE
// © Shlomi Steinberg, 2015

#pragma once

#include <streambuf>
#include <iosfwd>
#include <cstdlib>
#include <lib/string.hpp>
#include <lib/unique_ptr.hpp>
#include <array>

#include <log_sink.hpp>

namespace ste::log {

template <std::size_t buff_sz>
class log_streambuf : public std::streambuf {
public:
	using sink_type = log_sink;

private:
	bool force_flush;

	int_type overflow(int_type ch) override {
		sync();
		if (ch != traits_type::eof()) {
			*pptr() = ch;
			return ch;
		}

		return traits_type::eof();
	}

	int sync() override {
		char *p, *e;
		char *s = pbase();
		for (p = pbase(), e = pptr(); p != e; ++p) {
			if (*p == '\n') {
				lib::string line = p > s ? lib::string(s, static_cast<std::size_t>(p - s)) : lib::string();
				(*sink) << std::move(line);

				s = p + 1;
			}
		}

		if (s < e) {
			lib::string line(s, static_cast<std::size_t>(e - s));
			(*sink) << std::move(line);
		}

		std::ptrdiff_t n = e - pbase();
		pbump(static_cast<int>(-n));

		return 0;
	}

// 	std::streamsize xsputn(const char* s, std::streamsize n) {
// 		auto ret = std::streambuf::xsputn(s, n);
// 		if (force_flush) sync();
// 		return ret;
// 	}

private:
	lib::unique_ptr<sink_type> sink;
	std::array<char, buff_sz + 1> buffer;

public:
	log_streambuf(lib::unique_ptr<sink_type> &&sink, bool force_flush) : force_flush(force_flush), sink(std::move(sink)) {
		char *base = &buffer.front();
		setp(base, base + buffer.size());
	}
	log_streambuf(log_streambuf &&other) noexcept : std::streambuf(std::move(other)),
													force_flush(other.force_flush),
													sink(std::move(other.sink)),
													buffer(std::move(other.buffer)) {}
};

}
