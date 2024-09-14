#pragma once

#include <v8helper.h>
#include <optional>
#include <string>

namespace io
{
    std::optional<std::string> ReadFile(std::string_view path);
    std::string ReadFilePipe(FILE* pipe);
    void Print(v8helper::FunctionContext& ctx);
} // namespace io
