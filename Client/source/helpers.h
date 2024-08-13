#pragma once

#include <string_view>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include <fw/Logger.h>
#include <v8helper.h>
#include <v8.h>

inline void Assert(bool condition, std::string_view error, bool terminate = true)
{
    if (!condition)
    {
        fw::Logger::Get("JS")->Error("{}", error);
        if (terminate)
        {
            std::terminate();
        }
    }
}

inline std::optional<std::string> ReadFile(std::string_view path)
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

inline void Print(v8helper::FunctionContext& ctx)
{
    if (ctx.GetArgCount() == 0)
    {
        return;
    }

    std::stringstream buffer;
    for (int i = 0, argsCount = ctx.GetArgCount(); i < argsCount; i++)
    {
        v8::Local<v8::Value> val = ctx.GetArg<v8::Local<v8::Value>>(i);
        if (!val.IsEmpty())
        {
            buffer << v8helper::Stringify(val);

            if (i != argsCount - 1)
            {
                buffer << " ";
            }
        }
    }

    std::string message = buffer.str();
    if (message.empty())
    {
        fw::Logger::Get("JS")->Warn("{}", "April-snow-flake: could not print");
    }
    else
    {
        fw::Logger::Get("JS")->Info("{}", message);
    }
}

static std::string ReadFilePipe(FILE* pipe)
{
    constexpr size_t chunkSize = 1024;
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
