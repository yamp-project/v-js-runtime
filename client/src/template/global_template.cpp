#include "global_template.h"

namespace js::global {
    GlobalTemplate::GlobalTemplate(v8::Isolate* isolate, Logger* logger) : m_Isolate(isolate), m_Logger(logger) {

    }

    GlobalTemplate::~GlobalTemplate() = default;

    v8::Local<v8::ObjectTemplate> GlobalTemplate::GetGlobalTemplate() {
        v8::HandleScope handleScope(m_Isolate);
        m_Template = v8::ObjectTemplate::New(m_Isolate);

        // Temporary log function
        m_Template->Set(
            v8::String::NewFromUtf8Literal(m_Isolate, "yamp_log"),
            v8::FunctionTemplate::New(m_Isolate, LogCallback)
        );

        return m_Template;
    }

    void GlobalTemplate::LogCallback(const v8::FunctionCallbackInfo<v8::Value> &args) {
        v8::Local<v8::External> data = v8::Local<v8::External>::Cast(args.Data());
        auto* selfRef = static_cast<GlobalTemplate*>(data->Value());

        if (args.Length() > 0) {
            v8::String::Utf8Value utf8(args.GetIsolate(), args[0]);
            selfRef->m_Logger->Info("Log from resource: %s", *utf8);
        }
    }
}
