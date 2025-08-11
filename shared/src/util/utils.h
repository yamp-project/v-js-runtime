#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <cctype>
#include <fstream>

#define MAX_STR_SIZE = 64;

namespace utils
{
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
