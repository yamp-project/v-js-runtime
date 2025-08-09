#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <cctype>
#include <fstream>

#define MAX_STR_SIZE = 64;

namespace utils
{
    inline std::string StrToCamelCase(const char* input)
    {
        if (input == nullptr) {
            return {};
        }

        bool to_upper = false;
        std::string result;

        for (size_t i = 0; input[i] != '\0'; ++i) {
            if (i >= MAX_STR_SIZE) {
                break;
            }

            char ch = input[i];
            if (ch == '_') {
                to_upper = true;
                continue;
            }

            if (result.empty()) {
                result += std::tolower(static_cast<unsigned char>(ch));
            } else if (to_upper) {
                result += std::toupper(static_cast<unsigned char>(ch));
                to_upper = false;
            } else {
                result += std::tolower(static_cast<unsigned char>(ch));
            }
        }

        return result;
    }

    inline std::string ReadFile(const std::string& path) {
        std::ifstream file(path.c_str());
        if (!file.is_open()) {
            return "";
        }

        std::string content((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

        return content;
    }

    inline v8::Local<v8::String> StringToV8(v8::Isolate* isolate, const std::string& str) {
        return v8::String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked();
    }
}

#endif //UTILS_H
