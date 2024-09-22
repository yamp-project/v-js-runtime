#pragma once

namespace io
{
    std::string ReadFilePipe(FILE* pipe);
    std::optional<std::string> ReadFile(std::string_view path, bool isTypescript = false);
} // namespace io
