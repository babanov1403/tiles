#pragma once

#include <string>
#include <string_view>

namespace libhello {

std::string hello();

std::string hello(std::string_view user);

} // namespace libhello
