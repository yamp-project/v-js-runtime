#ifndef LOGGER_H
#define LOGGER_H

#include <string_view>
#include <yamp-sdk/sdk.h>
#include <format>
#include <utility>

#define IMPLEMENT_LOG(name)                                                                                                                 \
template <typename... Args>                                                                                                                 \
    void name(std::string_view fmt, Args&&... args)                                                                                         \
    {                                                                                                                                       \
        m_LookupTable->Log##name(m_Prefix.empty() ? fmt.data() : std::format("{} {}", m_Prefix, fmt).c_str(), std::forward<Args>(args)...); \
    }

class Logger
{
public:
    Logger(SDK_Interface* lookup): m_LookupTable(lookup)
    {

    }

    Logger(SDK_Interface* lookup, std::string  prefix): m_LookupTable(lookup), m_Prefix(std::move(prefix))
    {

    }

    IMPLEMENT_LOG(Debug);
    IMPLEMENT_LOG(Info);
    IMPLEMENT_LOG(Warn);
    IMPLEMENT_LOG(Error);

private:
    SDK_Interface* m_LookupTable;
    std::string m_Prefix;
};

#endif //LOGGER_H
