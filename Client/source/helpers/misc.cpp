#include "misc.h"

namespace helpers
{
    void Assert(bool condition, std::string_view error, bool terminate)
    {
        if (!condition)
        {
            // fw::Logger::Get("JS")->Error("{}", error);
            if (terminate)
            {
                std::terminate();
            }
        }
    }

    std::string ToCamelCase(std::string_view string)
    {
        std::string out;
        for (size_t i = 0, l = string.size(); i < l; ++i)
        {
            if (string[i] == '_')
            {
                out += std::toupper(string[++i]);
            }
            else
            {
                out += std::tolower(string[i]);
            }
        }

        return out;
    }
} // namespace helpers
