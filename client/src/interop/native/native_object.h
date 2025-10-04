#ifndef V_JS_RUNTIME_INTEROP_NATIVE_H
#define V_JS_RUNTIME_INTEROP_NATIVE_H
#include <JavaScriptCore/JSObjectRef.h>
#include "../../core/resource.h"
#include "../../wrapper/js_object.h"

namespace js::interop {
#pragma pack(push, 1)
    typedef struct {
        float x;
        float __padding_x;
        float y;
        float __padding_y;
        float z;
        float __padding_z;
    } NVector32;
    #pragma pack(pop)

    struct ArgPtr {
        void* m_Ptr;
        CNativeValueType m_Type;
    };

    class NativeObject final {
    public:
        static void Initialize(wrapper::JsObject* state);
    private:
        static void PushArg(JSContextRef ctx, JSValueRef arg, CNativeInvoker* invoker, CNativeValueType type, JSValueRef* exception);
        static ArgPtr PushArgPtr(CNativeInvoker* invoker, const CNativeValueType type);
        static JSValueRef PushValue(JSContextRef ctx, CNativeInvoker* invoker, CNativeValueType type);
        static JSValueRef PushValuePtr(JSContextRef ctx, CNativeInvoker* invoker, CNativeValueType type, void* buffer);

        static JSValueRef c_OnCall(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                                   size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
    };
}

#endif //V_JS_RUNTIME_INTEROP_NATIVE_H