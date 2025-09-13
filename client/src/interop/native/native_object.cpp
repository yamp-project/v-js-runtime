#include "native_object.h"

#include "../../core/runtime.h"
#include "../../util/js_error_util.h"

namespace js::interop {
    template<typename T>
    static T* CreatePtr(T value, const size_t additionalSize = 1) {
        T* ptr = static_cast<T*>(malloc(additionalSize * sizeof(T)));
        if (ptr) {
            *ptr = value;
        }
        return ptr;
    }

    template<typename T>
    static T* CreatePtr(const size_t additionalSize = 1) {
        return static_cast<T*>(malloc(additionalSize * sizeof(T)));
    }

    template<typename T>
    static void PushArgumentAsType(CNativeInvoker* invoker, T&& value) {
        auto lookupTable = Runtime::GetInstance()->GetLookupTable();
        lookupTable->nativesFactory->PushArgumentFromBuffer(invoker, &value, sizeof(T));
    }

    JSClassRef NativeObject::s_ResourceClass = nullptr;

    JSStaticValue NativeObject::s_Properties[] = {
        {nullptr, nullptr, nullptr, 0},
    };

    JSStaticFunction NativeObject::s_Functions[] = {
        {"native", c_OnCall, kJSPropertyAttributeReadOnly},
        {nullptr, nullptr, 0}
    };

    JSClassRef NativeObject::GetClass() {
        if (s_ResourceClass == nullptr) {
            JSClassDefinition classDefinition = kJSClassDefinitionEmpty;
            classDefinition.staticValues = s_Properties;
            classDefinition.staticFunctions = s_Functions;
            classDefinition.className = "Native";
            classDefinition.initialize = nullptr;
            classDefinition.finalize = nullptr;

            s_ResourceClass = JSClassCreate(&classDefinition);
        }
        return s_ResourceClass;
    }

    JSObjectRef NativeObject::CreateNativeObject(JSContextRef ctx) {
        JSClassRef nativeClass = GetClass();
        return JSObjectMake(ctx, nativeClass, nullptr);
    }

    void NativeObject::PushArg(JSContextRef ctx, JSValueRef arg, CNativeInvoker* invoker, CNativeValueType type, JSValueRef* exception) {
        if (JSValueIsBoolean(ctx, arg)) {
            PushArgumentAsType(invoker, JSValueToBoolean(ctx, arg));
        }
        else if (JSValueIsNumber(ctx, arg)) {
            switch (type) {
                case FLOAT_TYPE: {
                    double numberValue = JSValueToNumber(ctx, arg, exception);
                    if (*exception) return;
                    PushArgumentAsType(invoker, static_cast<float>(numberValue));
                    break;
                }
                case INT_TYPE: {
                    double numberValue = JSValueToNumber(ctx, arg, exception);
                    if (*exception) return;
                    PushArgumentAsType(invoker, static_cast<int>(numberValue));
                    break;
                }
                case UINT_TYPE: {
                    double numberValue = JSValueToNumber(ctx, arg, exception);
                    if (*exception) return;
                    PushArgumentAsType(invoker, static_cast<unsigned int>(numberValue));
                    break;
                }
                default: break;
            }
        }
        else if (JSValueIsString(ctx, arg)) {
            JSStringRef stringRef = JSValueToStringCopy(ctx, arg, exception);
            if (*exception) return;

            size_t maxSize = JSStringGetMaximumUTF8CStringSize(stringRef);
            char* buffer = new char[maxSize];
            JSStringGetUTF8CString(stringRef, buffer, maxSize);
            std::string stringValue(buffer);
            delete[] buffer;
            JSStringRelease(stringRef);

            PushArgumentAsType(invoker, stringValue);
        }
        else if (JSValueIsObject(ctx, arg)) {
            JSObjectRef objRef = JSValueToObject(ctx, arg, exception);
            if (*exception) return;

            JSStringRef lengthStr = JSStringCreateWithUTF8CString("length");
            JSValueRef lengthVal = JSObjectGetProperty(ctx, objRef, lengthStr, exception);
            JSStringRelease(lengthStr);

            if (!*exception && JSValueIsNumber(ctx, lengthVal)) {
                double length = JSValueToNumber(ctx, lengthVal, exception);
                if (!*exception && length >= 3) {
                    JSValueRef xVal = JSObjectGetPropertyAtIndex(ctx, objRef, 0, exception);
                    JSValueRef yVal = JSObjectGetPropertyAtIndex(ctx, objRef, 1, exception);
                    JSValueRef zVal = JSObjectGetPropertyAtIndex(ctx, objRef, 2, exception);

                    if (!*exception) {
                        float x = static_cast<float>(JSValueToNumber(ctx, xVal, exception));
                        float y = static_cast<float>(JSValueToNumber(ctx, yVal, exception));
                        float z = static_cast<float>(JSValueToNumber(ctx, zVal, exception));

                        if (!*exception) {
                            PushArgumentAsType(invoker, NVector32{x, 0, y, 0, z, 0});
                        }
                    }
                }
            }
        }
    }

    ArgPtr NativeObject::PushArgPtr(CNativeInvoker* invoker, const CNativeValueType type) {
        auto lookupTable = Runtime::GetInstance()->GetLookupTable();
        void* buffer = nullptr;

        switch (type) {
            case BOOL_TYPE: {
                buffer = CreatePtr<bool>();
                lookupTable->nativesFactory->PushArgumentFromBuffer(invoker, reinterpret_cast<bool*>(&buffer), sizeof(bool*));
                break;
            }
            case FLOAT_TYPE: {
                buffer = CreatePtr<float>();
                lookupTable->nativesFactory->PushArgumentFromBuffer(invoker, reinterpret_cast<float*>(&buffer), sizeof(float*));
                break;
            }
            case INT_TYPE: {
                buffer = CreatePtr<int>();
                lookupTable->nativesFactory->PushArgumentFromBuffer(invoker, reinterpret_cast<int*>(&buffer), sizeof(int*));
                break;
            }
            case UINT_TYPE: {
                buffer = CreatePtr<unsigned int>();
                lookupTable->nativesFactory->PushArgumentFromBuffer(invoker, reinterpret_cast<unsigned int*>(&buffer), sizeof(unsigned int*));
                break;
            }
            case VECTOR_TYPE: {
                buffer = CreatePtr<NVector32>();
                lookupTable->nativesFactory->PushArgumentFromBuffer(invoker, reinterpret_cast<NVector32*>(&buffer), sizeof(NVector32*));
                break;
            }
            case STRING_TYPE: {
                buffer = CreatePtr<char>();
                lookupTable->nativesFactory->PushArgumentFromBuffer(invoker, reinterpret_cast<char*>(&buffer), sizeof(char*));
                break;
            }
            default: break;
        }

        return { buffer, type };
    }

    JSValueRef NativeObject::PushValue(JSContextRef ctx, CNativeInvoker* invoker, CNativeValueType type) {
        auto lookupTable = Runtime::GetInstance()->GetLookupTable();

        switch (type) {
            case BOOL_TYPE: {
                bool value = *static_cast<bool*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker));
                return JSValueMakeBoolean(ctx, value);
            }
            case FLOAT_TYPE: {
                float value = *static_cast<float*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker));
                return JSValueMakeNumber(ctx, value);
            }
            case INT_TYPE: {
                int value = *static_cast<int*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker));
                return JSValueMakeNumber(ctx, value);
            }
            case UINT_TYPE: {
                unsigned int value = *static_cast<unsigned int*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker));
                return JSValueMakeNumber(ctx, value);
            }
            case VECTOR_TYPE: {
                NVector32 vector = *static_cast<NVector32*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker));

                JSValueRef arrayElements[6];
                arrayElements[0] = JSValueMakeNumber(ctx, vector.x);
                arrayElements[1] = JSValueMakeNumber(ctx, vector.__padding_x);
                arrayElements[2] = JSValueMakeNumber(ctx, vector.y);
                arrayElements[3] = JSValueMakeNumber(ctx, vector.__padding_y);
                arrayElements[4] = JSValueMakeNumber(ctx, vector.z);
                arrayElements[5] = JSValueMakeNumber(ctx, vector.__padding_z);

                return JSObjectMakeArray(ctx, 6, arrayElements, nullptr);
            }
            case STRING_TYPE: {
                std::string value = *static_cast<std::string*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker));
                JSStringRef jsString = JSStringCreateWithUTF8CString(value.c_str());
                JSValueRef result = JSValueMakeString(ctx, jsString);
                JSStringRelease(jsString);
                return result;
            }
            default:
                return JSValueMakeUndefined(ctx);
        }
    }

    JSValueRef NativeObject::PushValuePtr(JSContextRef ctx, CNativeInvoker* invoker, CNativeValueType type, void* buffer) {
        switch (type) {
            case BOOL_TYPE: {
                bool value = *static_cast<bool*>(buffer);
                return JSValueMakeBoolean(ctx, value);
            }
            case FLOAT_TYPE: {
                float value = *static_cast<float*>(buffer);
                return JSValueMakeNumber(ctx, value);
            }
            case INT_TYPE: {
                int value = *static_cast<int*>(buffer);
                return JSValueMakeNumber(ctx, value);
            }
            case UINT_TYPE: {
                unsigned int value = *static_cast<unsigned int*>(buffer);
                return JSValueMakeNumber(ctx, value);
            }
            case VECTOR_TYPE: {
                NVector32 vector = *static_cast<NVector32*>(buffer);

                JSValueRef arrayElements[6];
                arrayElements[0] = JSValueMakeNumber(ctx, vector.x);
                arrayElements[1] = JSValueMakeNumber(ctx, vector.__padding_x);
                arrayElements[2] = JSValueMakeNumber(ctx, vector.y);
                arrayElements[3] = JSValueMakeNumber(ctx, vector.__padding_y);
                arrayElements[4] = JSValueMakeNumber(ctx, vector.z);
                arrayElements[5] = JSValueMakeNumber(ctx, vector.__padding_z);

                return JSObjectMakeArray(ctx, 6, arrayElements, nullptr);
            }
            case STRING_TYPE: {
                char* stringValue = static_cast<char*>(buffer);
                if (stringValue) {
                    JSStringRef jsString = JSStringCreateWithUTF8CString(stringValue);
                    JSValueRef result = JSValueMakeString(ctx, jsString);
                    JSStringRelease(jsString);
                    return result;
                }
                return JSValueMakeString(ctx, JSStringCreateWithUTF8CString(""));
            }
            default:
                return JSValueMakeUndefined(ctx);
        }
    }

    JSValueRef NativeObject::c_OnCall(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                                      size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception) {

        if (argumentCount < 1) {
            return util::CreateException(ctx, exception, "native() requires at least 1 argument: native hash");
        }

        auto lookupTable = Runtime::GetInstance()->GetLookupTable();
        auto nativeRef = lookupTable->nativesFactory->CreateReflection();

        uint64_t nativeHash = static_cast<uint64_t>(JSValueToNumber(ctx, arguments[0], exception));
        if (*exception) return JSValueMakeUndefined(ctx);

        const CNativeInformation* nativeInformation = lookupTable->nativesFactory->GetNativeInformation(nativeRef, nativeHash);
        if (nativeInformation == nullptr) {
            return util::CreateException(ctx, exception, "Native not found");
        }

        CNativeInvoker* invoker = lookupTable->nativesFactory->CreateInvoker();
        if (!invoker) {
            return util::CreateException(ctx, exception, "Failed to create invoker");
        }

        std::vector<ArgPtr> returnPtrs;
        lookupTable->nativesFactory->Begin(invoker, nativeInformation->m_Hash);

        int skippingArgPtr = 0;

        for (int i = 0; i < nativeInformation->m_ParameterValueArraySize && (i + 1) < argumentCount; i++) {
            CNativeValueInformation nativeParameter = nativeInformation->m_ParameterValueArrayData[i];
            if (!nativeParameter.m_IsPointer) {
                PushArg(ctx, arguments[i + 1], invoker, nativeParameter.m_Type, exception);
                if (*exception) return JSValueMakeUndefined(ctx);
            }
            else {
                returnPtrs.push_back(PushArgPtr(invoker, nativeParameter.m_Type));
                skippingArgPtr++;
            }
        }

        for (int x = skippingArgPtr; x < nativeInformation->m_ParameterValueArraySize; x++) {
            auto [m_Type, m_IsPointer] = nativeInformation->m_ParameterValueArrayData[x];
            if (nativeInformation->m_ParameterValueArrayData[x].m_IsPointer) {
                returnPtrs.push_back(PushArgPtr(invoker, m_Type));
            }
        }

        lookupTable->nativesFactory->Call(invoker);

        size_t arraySize = returnPtrs.size() + 2;
        JSValueRef* arrayElements = new JSValueRef[arraySize];

        arrayElements[0] = PushValue(ctx, invoker, nativeInformation->m_ReturnValue.m_Type);

        for (int i = 0; i < returnPtrs.size(); i++) {
            ArgPtr ret = returnPtrs[i];
            arrayElements[i + 1] = PushValuePtr(ctx, invoker, ret.m_Type, ret.m_Ptr);
            free(ret.m_Ptr);
        }

        // Return count
        const int defaultReturn = nativeInformation->m_ReturnValue.m_Type != VOID_TYPE ? 1 : 0;
        arrayElements[returnPtrs.size() + 1] = JSValueMakeNumber(ctx, defaultReturn + returnPtrs.size());

        JSValueRef result = JSObjectMakeArray(ctx, arraySize, arrayElements, exception);
        delete[] arrayElements;

        return result;
    }
}