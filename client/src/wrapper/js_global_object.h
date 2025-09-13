#ifndef V_JS_RUNTIME_GLOBAL_OBJECT_H
#define V_JS_RUNTIME_GLOBAL_OBJECT_H
#include "../interop/resource/resource_object.h"
#include "resource.h"

namespace js::wrapper {
    static void SetupGlobalObjects(JSGlobalContextRef context, Resource* resource) {
        JSObjectRef globalObject = JSContextGetGlobalObject(context);

        JSObjectRef yampObject = JSObjectMake(context, nullptr, nullptr);

        JSObjectRef resourceObject = interop::ResourceObject::CreateResourceObject(context, resource);

        // add resource object to yamp object
        JSStringRef resourceName = JSStringCreateWithUTF8CString("resource");
        JSObjectSetProperty(context, yampObject, resourceName, resourceObject, kJSPropertyAttributeReadOnly, nullptr);
        JSStringRelease(resourceName);

        // Add yamp object to global
        JSStringRef yampName = JSStringCreateWithUTF8CString("yamp");
    JSObjectSetProperty(context, globalObject, yampName, yampObject, kJSPropertyAttributeReadOnly, nullptr);
        JSStringRelease(yampName);
    }
}

#endif //V_JS_RUNTIME_GLOBAL_OBJECT_H