#include "resource_object.h"

#include "../../util/js_error_util.h"

namespace js::interop {
    JSClassRef ResourceObject::s_ResourceClass = nullptr;

    JSStaticValue ResourceObject::s_Properties[] = {
        {"name", c_GetName, nullptr, kJSPropertyAttributeReadOnly},
        {"resourcePath", c_GetResourcePath, nullptr, kJSPropertyAttributeReadOnly},
        {"resourceMainFile", c_GetResourceMainFile, nullptr, kJSPropertyAttributeReadOnly},
        {nullptr, nullptr, nullptr, 0},
    };

    JSStaticFunction ResourceObject::s_Functions[] = {
        {"on", c_OnEvent, kJSPropertyAttributeReadOnly},
        {nullptr, nullptr, 0}
    };

    JSClassRef ResourceObject::GetResourceClass() {
        if (s_ResourceClass == nullptr) {
            JSClassDefinition classDefinition = kJSClassDefinitionEmpty;
            classDefinition.staticValues = s_Properties;
            classDefinition.staticFunctions = s_Functions;
            classDefinition.className = "Resource";
            classDefinition.initialize = nullptr;
            classDefinition.finalize = nullptr;

            s_ResourceClass = JSClassCreate(&classDefinition);
        }

        return s_ResourceClass;
    }

    JSObjectRef ResourceObject::CreateResourceObject(JSContextRef ctx, Resource *resource) {
        JSClassRef resourceClass = GetResourceClass();
        return JSObjectMake(ctx, resourceClass, resource);
    }

    Resource * ResourceObject::GetResourceFromObject(JSObjectRef object) {
        return static_cast<Resource *>(JSObjectGetPrivate(object));
    }

    JSValueRef ResourceObject::c_GetName(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName,
                                         JSValueRef* exception) {
        Resource* resource = GetResourceFromObject(object);
        if (!resource) {
            return util::CreateException(ctx, exception, "Resource not available!");
        }

        std::string name = resource->GetResourceName();

        JSStringRef nameString = JSStringCreateWithUTF8CString(name.c_str());
        JSValueRef result = JSValueMakeString(ctx, nameString);
        JSStringRelease(nameString);
        return result;
    }

    JSValueRef ResourceObject::c_GetResourcePath(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName,
        JSValueRef* exception) {
        Resource* resource = GetResourceFromObject(object);
        if (!resource) {
            return util::CreateException(ctx, exception, "Resource not available!");
        }

        std::string path = resource->GetResourceName();

        JSStringRef pathString = JSStringCreateWithUTF8CString(path.c_str());
        JSValueRef result = JSValueMakeString(ctx, pathString);
        JSStringRelease(pathString);
        return result;
    }

    JSValueRef ResourceObject::c_GetResourceMainFile(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName,
        JSValueRef* exception) {
        Resource* resource = GetResourceFromObject(object);
        if (!resource) {
            return util::CreateException(ctx, exception, "Resource not available!");
        }

        std::string mainFile = resource->GetMainFilePath();

        JSStringRef mainFileString = JSStringCreateWithUTF8CString(mainFile.c_str());
        JSValueRef result = JSValueMakeString(ctx, mainFileString);
        JSStringRelease(mainFileString);
        return result;
    }

    JSValueRef ResourceObject::c_OnEvent(JSContextRef ctx, JSObjectRef function, JSObjectRef object,
        size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception) {
        Resource* resource = GetResourceFromObject(object);
        if (!resource) {
            return util::CreateException(ctx, exception, "Resource not available!");
        }

        if (argumentCount < 2) {
            return util::CreateException(ctx, exception, "yamp.resource.on() requires 2 arguments: eventName and callback");
        }

        JSStringRef eventNameRef = JSValueToStringCopy(ctx, arguments[0], exception);
        if (*exception) {
            return JSValueMakeUndefined(ctx);
        }

        size_t maxSize = JSStringGetMaximumUTF8CStringSize(eventNameRef);
        const auto eventNameBuffer = new char[maxSize];
        JSStringGetUTF8CString(eventNameRef, eventNameBuffer, maxSize);
        std::string eventName(eventNameBuffer);
        delete[] eventNameBuffer;
        JSStringRelease(eventNameRef);

        JSObjectRef callback = JSValueToObject(ctx, arguments[1], exception);
        if (*exception) {
            return JSValueMakeUndefined(ctx);
        }

        const std::optional<CoreEventType> coreEvent = Runtime::GetInstance()->GetCoreEventType(eventName.c_str());
        if (coreEvent.has_value()) {
            resource->AddCoreEventCallback(coreEvent.value(), callback);
        } else {
            resource->AddEventCallback(eventName, callback);
        }

        return JSValueMakeUndefined(ctx);
    }
}
