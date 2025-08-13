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

    inline v8::Local<v8::Value> ToV8Value(v8::Isolate* isolate, const CAnyValue& anyValue) {
        switch (static_cast<CType>(anyValue.type)) {
            case C_INT_8: {
                return v8::Number::New(isolate, anyValue.value.m_Int8);
            }
            case C_INT_16: {
                return v8::Number::New(isolate, anyValue.value.m_Int16);
            }
            case C_INT_32: {
                return v8::Number::New(isolate, anyValue.value.m_Int32);
            }
            case C_INT_64: {
                return v8::Number::New(isolate, anyValue.value.m_Int64);
            }
            case C_UINT_8: {
                return v8::Number::New(isolate, anyValue.value.m_Uint8);
            }
            case C_UINT_16: {
                return v8::Number::New(isolate, anyValue.value.m_Uint16);
            }
            case C_UINT_32: {
                return v8::Number::New(isolate, anyValue.value.m_Uint32);
            }
            case C_UINT_64: {
                return v8::Number::New(isolate, anyValue.value.m_Uint64);
            }
            case C_DOUBLE: {
                return v8::Number::New(isolate, anyValue.value.m_Double);
            }
            case C_NULL: {
                // TODO
            }
            case C_STRING: {
                return StringToV8(isolate, anyValue.value.m_String);
            }
            case C_FLOAT: {
                return v8::Number::New(isolate, anyValue.value.m_Float);
            }
            case C_BOOL: {
                return v8::Boolean::New(isolate, anyValue.value.m_Bool);
            }
            case C_ANY_ARRAY: {
                // TODO
            }
            case C_ARRAY: {
                // TODO
            }
            case C_DICT: {
                // TODO
            }
            case C_VECTOR: {
                // TODO
            }
            case C_DATE: {
                // TODO
            }
            default: return {};
        }
    }

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

            const char ch = input[i];
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

    inline float V8ValueToFloat(v8::Isolate* isolate, const v8::Local<v8::Value> value) {
        if (!value->IsNumber()) {
            return 0.0f;
        }

        return static_cast<float>(value->NumberValue(isolate->GetCurrentContext()).ToChecked());
    }
}

#endif //UTILS_H
