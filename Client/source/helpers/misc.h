#pragma once

#include <string>

namespace helpers
{
    void Assert(bool condition, std::string_view error, bool terminate = true);
    std::string ToCamelCase(std::string_view string);
} // namespace helpers
