#include "files.h"

namespace io
{
    std::optional<std::string> ReadFile(std::string_view path)
    {
        if (!std::filesystem::exists(path))
        {
            return std::nullopt;
        }

        std::ifstream file(path.data());
        if (!file.is_open())
        {
            return std::nullopt;
        }

        std::stringstream stream;
        stream << file.rdbuf();
        return stream.str();
    }

    std::string ReadFilePipe(FILE* pipe)
    {
        constexpr uint16_t chunkSize = 1024;
        std::vector<char> buffer;
        buffer.reserve(chunkSize);

        char tempBuffer[chunkSize];
        size_t bytesRead;

        while ((bytesRead = fread(tempBuffer, 1, chunkSize, pipe)) > 0)
        {
            buffer.insert(buffer.end(), tempBuffer, tempBuffer + bytesRead);
        }

        return std::string(buffer.data(), buffer.size());
    }
} // namespace io
