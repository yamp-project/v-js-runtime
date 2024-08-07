#pragma once

#include <string_view>
#include <filesystem>
#include <iostream>
#include <fstream>

#include <fw/Logger.h>
#include <v8helper.h>
#include <v8.h>

inline void Assert(bool cond, std::string_view error, bool terminate = true)
{
    if (!cond)
    {
        std::cerr << error << std::endl;
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

    for (int i = 0; i < ctx.GetArgCount(); i++)
    {
        v8::Local<v8::Value> val = ctx.GetArg<v8::Local<v8::Value>>(i);
        if (val.IsEmpty())
        {
            continue;
        }

        fw::Logger::Get("JS")->Info("{}", v8helper::Stringify(val));
        if (i != ctx.GetArgCount())
        {
            fw::Logger::Get("JS")->Info(" ");
        }
    }
}
