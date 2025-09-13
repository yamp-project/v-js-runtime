#ifndef V_JS_RUNTIME_INTEROP_RESOURCE_H
#define V_JS_RUNTIME_INTEROP_RESOURCE_H
#include <JavaScriptCore/JSObjectRef.h>
#include "../../core/resource.h"

namespace js::interop {
    class ResourceObject final {
    public:
        static JSStaticValue* GetProperties() {
            return s_Properties;
        }

        static JSStaticFunction* GetFunctions() {
            return s_Functions;
        }

        static JSClassRef GetResourceClass();
        static JSObjectRef CreateResourceObject(JSContextRef ctx, Resource* resource);
        static Resource* GetResourceFromObject(JSObjectRef object);
    private:
        static JSClassRef s_ResourceClass;
        static JSStaticValue s_Properties[];
        static JSStaticFunction s_Functions[];
    private:
        static JSValueRef c_GetName(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);
        static JSValueRef c_GetResourcePath(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);
        static JSValueRef c_GetResourceMainFile(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception);
        static JSValueRef c_OnEvent(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

    };
}

#endif //V_JS_RUNTIME_INTEROP_RESOURCE_H