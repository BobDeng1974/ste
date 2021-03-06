
#include <stdafx.hpp>
#include <kelvin.hpp>
#include <rgb.hpp>

using namespace ste::graphics;

rgb kelvin::toRGB() const {
	rgb ret;

	/*
	 *	Thanks to Tanner Helland:
	 *	http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
	*/

	float temperature = K() / 100.f;

	if (temperature <= 66)
		ret.R() = 255;
	else {
		ret.R() = temperature - 60;
		ret.R() = static_cast<T>(329.698727446 * glm::pow(ret.R(), -0.1332047592));
		if (ret.R() < 0) ret.R() = 0;
		if (ret.R() > 255) ret.R() = 255;
	}

	if (temperature <= 66) {
		ret.G() = temperature;
		ret.G() = static_cast<T>(99.4708025861 * glm::log(ret.G()) - 161.1195681661);
		if (ret.G() < 0) ret.G() = 0;
		if (ret.G() > 255) ret.G() = 255;
	}
	else {
		ret.G() = temperature - 60;
		ret.G() = static_cast<T>(288.1221695283 * glm::pow(ret.G(), -0.0755148492));
		if (ret.G() < 0) ret.G() = 0;
		if (ret.G() > 255) ret.G() = 255;
	}

	if (temperature >= 66)
		ret.B() = 255;
	else {
		if (temperature <= 19)
			ret.B() = 0;
		else {
			ret.B() = temperature - 10;
			ret.B() = static_cast<T>(138.5177312231 * glm::log(ret.B()) - 305.0447927307);
			if (ret.B() < 0) ret.B() = 0;
			if (ret.B() > 255) ret.B() = 255;
		}
	}

	ret.R() /= 255.f;
	ret.G() /= 255.f;
	ret.B() /= 255.f;

	return ret;
}

kelvin::operator rgb() const {
	return toRGB();
};
