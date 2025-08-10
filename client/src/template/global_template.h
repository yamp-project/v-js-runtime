#ifndef V8_GLOBAL_TEMPLATE_H
#define V8_GLOBAL_TEMPLATE_H
#include "v8.h"
#include "util/logger.h"

namespace js::global {
    class GlobalTemplate {
    public:
        GlobalTemplate(v8::Isolate* isolate, Logger* logger);
        ~GlobalTemplate();

        v8::Local<v8::ObjectTemplate> GetGlobalTemplate();
    private:
        v8::Local<v8::ObjectTemplate> m_Template;

        static void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        v8::Isolate* m_Isolate;
        Logger* m_Logger;
    };
}

#endif //V8_GLOBAL_TEMPLATE_H
