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

        m_CurrentClassDefinitions->staticValues = m_CurrentProperties.data();
        m_CurrentClassDefinitions->staticFunctions = m_CurrentFunctions.data();
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

    JsObjectReturnData JsObject::CreateJSObjects(const JSContextRef ctx, Resource* providedData) {
        std::map<std::string, JSObjectRef> objects;

        for (const auto &[name, data]: m_RegisteredClasses) {
            objects.insert({ name, JSObjectMake(ctx, data.get(), providedData) });
        }

        JsObjectReturnData returnData;
        returnData.dataBuffer = std::move(objects);

        // Create parent Object
        // All functions and getters are available via yamp.[name]
        JSClassDefinition parentObj;

        *m_CurrentClassDefinitions = kJSClassDefinitionEmpty;
        m_CurrentClassDefinitions->staticValues = m_GlobalProperties.data();
        m_CurrentClassDefinitions->staticFunctions = m_GlobalFunctions.data();
        m_CurrentClassDefinitions->className = m_CurrentClassName.c_str();
        m_CurrentClassDefinitions->initialize = nullptr;
        m_CurrentClassDefinitions->finalize = nullptr;

        returnData.parentObject = JSObjectMake(ctx, JSClassCreate(&parentObj), providedData);

        return returnData;
    }
}
