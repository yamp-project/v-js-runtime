#ifndef V_JS_RUNTIME_JSCONTEXTWRAPPER_H
#define V_JS_RUNTIME_JSCONTEXTWRAPPER_H

#include <JavaScriptCore/JavaScriptCore.h>

namespace js::wrapper {
    class JSContextWrapper {
    public:
        JSContextWrapper() : m_Context(JSGlobalContextCreate(nullptr)) {}

        JSContextWrapper(JSClassRef globalClass)
            : m_Context(JSGlobalContextCreateInGroup(nullptr, globalClass)) {}

        ~JSContextWrapper() {
            if (m_Context) {
                JSGlobalContextRelease(m_Context);
            }
        }

        JSGlobalContextRef get() { return m_Context; }

        JSContextWrapper(const JSContextWrapper&) = delete;
        JSContextWrapper& operator=(const JSContextWrapper&) = delete;

    private:
        JSGlobalContextRef m_Context;
    };
}

#endif //V_JS_RUNTIME_JSCONTEXTWRAPPER_H