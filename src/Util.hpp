#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "AbstractIndicator.hpp"

namespace Util {

	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);

	std::vector<std::string> split(const std::string &s, char delim);
}
