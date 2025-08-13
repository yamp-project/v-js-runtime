#include "global_template.h"

#include "object/native_class.h"
#include "object/resource_class.h"
#include "util/utils.h"

namespace js::templates {
    GlobalTemplate::GlobalTemplate(v8::Isolate* isolate, Logger* logger, Resource* resource) : m_Isolate(isolate), m_Logger(logger), m_Resource(resource) {

    }

    GlobalTemplate::~GlobalTemplate() = default;

    v8::Local<v8::ObjectTemplate> GlobalTemplate::GetGlobalTemplate() {
        v8::HandleScope handleScope(m_Isolate);
        m_Template = v8::ObjectTemplate::New(m_Isolate);

        // Native invoker class
        const v8::Local<v8::ObjectTemplate> nativeTemplate = NativeClass::CreateTemplate(m_Isolate);
        const v8::Local<v8::Object> nativeObject = nativeTemplate->NewInstance(m_Isolate->GetCurrentContext()).ToLocalChecked();

        m_Template->Set(
            utils::StringToV8(m_Isolate, NativeClass::Name()),
            nativeObject
        );

        // yamp object
        const v8::Local<v8::ObjectTemplate> yampTemplate = v8::ObjectTemplate::New(m_Isolate);
        const v8::Local<v8::Object> yampObject = yampTemplate->NewInstance(m_Isolate->GetCurrentContext()).ToLocalChecked();

        // Resource class
        const v8::Local<v8::ObjectTemplate> resourceTemplate = ResourceClass::CreateTemplate(m_Isolate);

        const v8::Local<v8::Object> resourceObject = resourceTemplate->NewInstance(m_Isolate->GetCurrentContext()).ToLocalChecked();

        resourceObject->SetInternalField(0, v8::External::New(m_Isolate, m_Resource));

        const v8::Maybe<bool> yampObjectReturn = yampObject->Set(
            m_Isolate->GetCurrentContext(),
            utils::StringToV8(m_Isolate, ResourceClass::Name()),
            resourceObject
        );

        if (yampObjectReturn.IsEmpty()) {
            m_Logger->Error("While assigning resource class to yamp object an error occurred");
        }

        m_Template->Set(
            utils::StringToV8(m_Isolate, "yamp"),
            yampObject
        );

        return m_Template;
    }
}
