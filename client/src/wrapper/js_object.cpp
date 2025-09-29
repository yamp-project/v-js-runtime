#include "js_object.h"

#include <ranges>
#include <JavaScriptCore/JSObjectRef.h>

namespace js::wrapper {
    void JsObject::BeginClass(const std::string &className) {
        if (m_ClassInit == true) {
            EndClass();
        }
        m_ClassInit = true;
        m_CurrentClassName = className;

        m_CurrentClassDefinitions = std::make_shared<JSClassDefinition>();

        *m_CurrentClassDefinitions = kJSClassDefinitionEmpty;
        m_CurrentClassDefinitions->staticValues = nullptr;
        m_CurrentClassDefinitions->staticFunctions = nullptr;
        m_CurrentClassDefinitions->className = m_CurrentClassName.c_str();
        m_CurrentClassDefinitions->initialize = nullptr;
        m_CurrentClassDefinitions->finalize = nullptr;
    }

    void JsObject::EndClass() {
        if (m_CurrentClassDefinitions == nullptr) {
            m_ClassInit = false;
            m_CurrentClassName = std::string();
            m_CurrentClassDefinitions = nullptr;
            return;
        }

        m_ClassInit = false;

        m_CurrentClassDefinitions.get()->staticValues = m_CurrentProperties.data();
        m_CurrentClassDefinitions.get()->staticFunctions = m_CurrentFunctions.data();
        const JSClassRef classRef = JSClassCreate(m_CurrentClassDefinitions.get());
        m_RegisteredClasses.insert({m_CurrentClassName, std::shared_ptr<OpaqueJSClass>(classRef, JSClassRelease)});
    }

    void JsObject::Setter(const std::string &interface, const JSObjectSetPropertyCallback callback) {
           m_CurrentProperties.push_back({
            interface.c_str(),
            nullptr,
            callback,
            kJSPropertyAttributeReadOnly
        });
    }

    void JsObject::Getter(const std::string &interface, const JSObjectGetPropertyCallback callback) {
        m_CurrentProperties.push_back({
         interface.c_str(),
         callback,
         nullptr,
        kJSPropertyAttributeNone
        });
    }

    void JsObject::Function(const std::string &interface, const JSObjectCallAsFunctionCallback callback, const bool classBound) {
        const JSStaticFunction function = {
            interface.c_str(),
            callback,
            kJSPropertyAttributeReadOnly
        };

        if (classBound) {
            m_CurrentFunctions.push_back(function);
        } else {
            m_GlobalFunctions.push_back(function);
        }
    }

    std::vector<JSObjectRef> JsObject::CreateJSObjects(const JSContextRef ctx, Resource* providedData) {
        std::vector<JSObjectRef> objects;

        objects.reserve(m_CurrentFunctions.size());

        for (const auto &val: m_RegisteredClasses | std::views::values) {
            objects.push_back(JSObjectMake(ctx, val.get(), providedData));
        }

        return objects;
    }
}
