#include "global_template.h"

#include "function/LogFunction.h"
#include "object/LogClass.h"
#include "util/utils.h"

namespace js::global {
    GlobalTemplate::GlobalTemplate(v8::Isolate* isolate, Logger* logger) : m_Isolate(isolate), m_Logger(logger) {

    }

    GlobalTemplate::~GlobalTemplate() = default;

    v8::Local<v8::ObjectTemplate> GlobalTemplate::GetGlobalTemplate() {
        v8::HandleScope handleScope(m_Isolate);
        m_Template = v8::ObjectTemplate::New(m_Isolate);

        v8::Local<v8::External> loggerRef = v8::External::New(m_Isolate, m_Logger);

        // Temporary log function
        m_Template->Set(
            utils::StringToV8(m_Isolate, templates::LogFunction::Name()),
            v8::FunctionTemplate::New(m_Isolate, templates::LogFunction::Callback, loggerRef)
        );

        // Temporary log class
        v8::Local<v8::ObjectTemplate> objectTemplate = templates::LogClass::CreateTemplate(m_Isolate);

        v8::Local<v8::Object> loggerObject = objectTemplate->NewInstance(m_Isolate->GetCurrentContext()).ToLocalChecked();

        loggerObject->SetInternalField(0, loggerRef);

        m_Template->Set(
            utils::StringToV8(m_Isolate, templates::LogClass::Name()),
            loggerObject
        );

        return m_Template;
    }
}
