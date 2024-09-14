#pragma once

#include <optional>
#include <string>

namespace io
{
    std::optional<std::string> ReadFile(std::string_view path);
    std::string ReadFilePipe(FILE* pipe);
} // namespace io
