#include "files.h"

#include "subprocess.h"

// TODO: get the path from the client
static const char* COMMAND_LINE[] = {
    "D:/.yamp/v-client/bin/runtimes/esbuild.exe", "D:/.yamp/v-client/bin/resources/js_test/main.ts", "--bundle", "--format=esm", "--platform=browser", "--external:@yamp/client", NULL};

namespace io
{
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

    std::optional<std::string> ReadFile(std::string_view path, bool isTypescript)
    {
        if (!std::filesystem::exists(path))
        {
            return std::nullopt;
        }

        if (!isTypescript)
        {
            std::ifstream file(path.data());
            if (!file.is_open())
            {
                return std::nullopt;
            }

            std::stringstream stream;
            stream << file.rdbuf();
            return stream.str();
        }

        subprocess_s process;
        int8_t options = subprocess_option_combined_stdout_stderr | subprocess_option_no_window;
        int32_t result = subprocess_create(COMMAND_LINE, options, &process);
        if (result != 0)
        {
            return std::nullopt;
        }

        return ReadFilePipe(subprocess_stdout(&process));
    }
} // namespace io
