#pragma once

#include "stdafx.h"

namespace io
{
    std::optional<std::string> ReadFile(std::string_view path);
    std::string ReadFilePipe(FILE* pipe);
} // namespace io
