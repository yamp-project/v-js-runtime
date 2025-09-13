#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <cctype>
#include <fstream>

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

    inline JSValueRef StringToJS(JSContextRef ctx, const std::string& str) {
        JSStringRef jsStr = JSStringCreateWithUTF8CString(str.c_str());
        JSValueRef value = JSValueMakeString(ctx, jsStr);
        JSStringRelease(jsStr);
        return value;
    }

    inline JSValueRef ToJSValue(JSContextRef ctx, const CAnyValue& anyValue) {
    switch (static_cast<CType>(anyValue.type)) {
        case C_INT_8: {
            return JSValueMakeNumber(ctx, anyValue.value.m_Int8);
        }
        case C_INT_16: {
            return JSValueMakeNumber(ctx, anyValue.value.m_Int16);
        }
        case C_INT_32: {
            return JSValueMakeNumber(ctx, anyValue.value.m_Int32);
        }
        case C_INT_64: {
            return JSValueMakeNumber(ctx, static_cast<double>(anyValue.value.m_Int64));
        }
        case C_UINT_8: {
            return JSValueMakeNumber(ctx, anyValue.value.m_Uint8);
        }
        case C_UINT_16: {
            return JSValueMakeNumber(ctx, anyValue.value.m_Uint16);
        }
        case C_UINT_32: {
            return JSValueMakeNumber(ctx, anyValue.value.m_Uint32);
        }
        case C_UINT_64: {
            return JSValueMakeNumber(ctx, static_cast<double>(anyValue.value.m_Uint64));
        }
        case C_DOUBLE: {
            return JSValueMakeNumber(ctx, anyValue.value.m_Double);
        }
        case C_FLOAT: {
            return JSValueMakeNumber(ctx, static_cast<double>(anyValue.value.m_Float));
        }
        case C_BOOL: {
            return JSValueMakeBoolean(ctx, anyValue.value.m_Bool);
        }
        case C_NULL: {
            return JSValueMakeNull(ctx);
        }
        case C_STRING: {
            return StringToJS(ctx, anyValue.value.m_String);
        }
        case C_ANY_ARRAY: {
            // TODO: Convert to JSArray
            return JSValueMakeNull(ctx);
        }
        case C_ARRAY: {
            // TODO: Convert to JSArray
            return JSValueMakeNull(ctx);
        }
        case C_DICT: {
            // TODO: Convert to JSObject
            return JSValueMakeNull(ctx);
        }
        case C_VECTOR: {
            // TODO: Convert to JSArray
            return JSValueMakeNull(ctx);
        }
        case C_DATE: {
            // TODO: Convert to JS Date object
            return JSValueMakeNull(ctx);
        }
        default:
            return JSValueMakeUndefined(ctx);
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
            if (i >= 64) {
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
}

#endif //UTILS_H
