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

        // yamp object
        v8::Local<v8::ObjectTemplate> yampTemplate = v8::ObjectTemplate::New(m_Isolate);
        v8::Local<v8::Object> yampObject = yampTemplate->NewInstance(m_Isolate->GetCurrentContext()).ToLocalChecked();

        // Resource class
        v8::Local<v8::ObjectTemplate> resourceTemplate = ResourceClass::CreateTemplate(m_Isolate);

        v8::Local<v8::Object> resourceObject = resourceTemplate->NewInstance(m_Isolate->GetCurrentContext()).ToLocalChecked();

        resourceObject->SetInternalField(0, v8::External::New(m_Isolate, m_Resource));

        v8::Maybe<bool> yampObjectReturn = yampObject->Set(
            m_Isolate->GetCurrentContext(),
            utils::StringToV8(m_Isolate, ResourceClass::Name()),
            resourceObject
        );

        if (yampObjectReturn.IsEmpty()) {
            m_Logger->Error("While assigning resource class to yamp object an error occoured");
        }

        m_Template->Set(
            utils::StringToV8(m_Isolate, "yamp"),
            yampObject
        );

        return m_Template;
    }
}
