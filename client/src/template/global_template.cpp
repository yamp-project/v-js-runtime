#include "global_template.h"

#include "function/LogFunction.h"
#include "util/utils.h"

namespace js::global {
    GlobalTemplate::GlobalTemplate(v8::Isolate* isolate, Logger* logger) : m_Isolate(isolate), m_Logger(logger) {

    }

    GlobalTemplate::~GlobalTemplate() = default;

    v8::Local<v8::ObjectTemplate> GlobalTemplate::GetGlobalTemplate() {
        v8::HandleScope handleScope(m_Isolate);
        m_Template = v8::ObjectTemplate::New(m_Isolate);

        v8::Local<v8::External> externalRef = v8::External::New(m_Isolate, this);

        // Temporary log function
        m_Template->Set(
            utils::StringToV8(m_Isolate, function::LogFunction::FunctionName()),
            v8::FunctionTemplate::New(m_Isolate, function::LogFunction::Callback, externalRef)
        );

        return m_Template;
    }
}
