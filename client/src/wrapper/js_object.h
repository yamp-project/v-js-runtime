#ifndef V_JS_RUNTIME_JSOBJECT_H
#define V_JS_RUNTIME_JSOBJECT_H
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <JavaScriptCore/JSBase.h>
#include <JavaScriptCore/JSObjectRef.h>

namespace js {
    class Resource;
}

namespace js::wrapper {
    struct JsObjectReturnData {
        std::map<std::string, JSObjectRef> dataBuffer;
        JSObjectRef parentObject;
    };

    class JsObject {
    public:
        void BeginClass(const std::string &className);
        void EndClass();

        void Setter(const std::string &interface, JSObjectSetPropertyCallback callback);
        void Getter(const std::string &interface, JSObjectGetPropertyCallback callback);
        void Function(const std::string &interface, JSObjectCallAsFunctionCallback callback, bool classBound = true);

        JsObjectReturnData CreateJSObjects(JSContextRef ctx, Resource* providedData);
    private:
        bool m_ClassInit = false;
        std::string m_CurrentClassName;
        std::shared_ptr<JSClassDefinition> m_CurrentClassDefinitions = nullptr;
        std::vector<JSStaticFunction> m_CurrentFunctions;
        std::vector<JSStaticValue> m_CurrentProperties;

        std::vector<JSStaticValue> m_GlobalProperties;
        std::vector<JSStaticFunction> m_GlobalFunctions;

        std::map<std::string, std::shared_ptr<OpaqueJSClass>> m_RegisteredClasses;
    };
}

#endif //V_JS_RUNTIME_JSOBJECT_H