#include "libhello/hello.h"

#include "absl/strings/str_cat.h"

namespace libhello {

std::string hello()
{
    return "Hello World!";
}

std::string hello(std::string_view user)
{
    return absl::StrCat("Hello, ", user, "!");
}

} // namespace libhello
