#include "global_template.h"

#include "function/LogFunction.h"
#include "object/ResourceClass.h"
#include "util/utils.h"

namespace js::templates {
    GlobalTemplate::GlobalTemplate(v8::Isolate* isolate, Logger* logger, Resource* resource) : m_Isolate(isolate), m_Logger(logger), m_Resource(resource) {

    }

    GlobalTemplate::~GlobalTemplate() = default;

    v8::Local<v8::ObjectTemplate> GlobalTemplate::GetGlobalTemplate() {
        v8::HandleScope handleScope(m_Isolate);
        m_Template = v8::ObjectTemplate::New(m_Isolate);

        // Temporary log function
        m_Template->Set(
            utils::StringToV8(m_Isolate, LogFunction::Name()),
            v8::FunctionTemplate::New(m_Isolate, LogFunction::Callback, v8::External::New(m_Isolate, m_Logger))
        );

        // Resource class
        v8::Local<v8::ObjectTemplate> objectTemplate = ResourceClass::CreateTemplate(m_Isolate);

        v8::Local<v8::Object> resourceObject = objectTemplate->NewInstance(m_Isolate->GetCurrentContext()).ToLocalChecked();

        resourceObject->SetInternalField(0, v8::External::New(m_Isolate, m_Resource));

        m_Template->Set(
            utils::StringToV8(m_Isolate, ResourceClass::Name()),
            resourceObject
        );

        return m_Template;
    }
}
