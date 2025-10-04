#ifndef V_JS_RUNTIME_JS_ERROR_UTIL_H
#define V_JS_RUNTIME_JS_ERROR_UTIL_H
#include <string>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSValueRef.h>

namespace js::util {
    static JSValueRef CreateException(JSContextRef ctx, JSValueRef* exception, const std::string &msg) {
        const JSStringRef errorString = JSStringCreateWithUTF8CString(msg.c_str());
        *exception = JSValueMakeString(ctx, errorString);
        JSStringRelease(errorString);
        return JSValueMakeUndefined(ctx);
    }
}

#endif //V_JS_RUNTIME_JS_ERROR_UTIL_H