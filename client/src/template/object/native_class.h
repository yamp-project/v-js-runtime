#ifndef V8_NATIVE_CLASS_H
#define V8_NATIVE_CLASS_H
#include <string>
#include <v8.h>

#include "../../core/runtime.h"
#include "util/utils.h"

namespace js::templates {
    #pragma pack(push, 1)
        typedef struct
        {
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

    template<typename T>
    T* CreatePtr(T value, const size_t additionalSize = 1) {
        T* ptr = static_cast<T*>(malloc(additionalSize * sizeof(T)));
        if (value && ptr) {
            *ptr = value;
        }

        return ptr;
    }

    template<typename T>
    T* CreatePtr(const size_t additionalSize = 1) {
        return static_cast<T*>(malloc(additionalSize * sizeof(T)));
    }

    template<typename T>
    void PushArgumentAsType(CNativeInvoker* invoker, T&& value) {
        auto lookupTable = Runtime::GetInstance()->GetLookupTable();
        lookupTable->nativesFactory->PushArgumentFromBuffer(invoker, &value, sizeof(T));
    }

    void PushArg(v8::Isolate* isolate, v8::Local<v8::Value> arg, CNativeInvoker* invoker, CNativeValueType type) {
        if (arg->IsBoolean()) {
            PushArgumentAsType(invoker, arg->BooleanValue(isolate));
        }
        else if (arg->IsNumber()) {
            switch (type) {
                case FLOAT_TYPE: {
                    PushArgumentAsType(invoker, arg->NumberValue(isolate->GetCurrentContext()));
                    break;
                }
                case INT_TYPE: {
                    PushArgumentAsType(invoker, arg->IntegerValue(isolate->GetCurrentContext()));
                    break;
                }
                case UINT_TYPE: {
                    PushArgumentAsType(invoker, arg->Uint32Value(isolate->GetCurrentContext()));
                    break;
                }
                default: break;
            }
        }
        else if (arg->IsString()) {
            PushArgumentAsType(invoker, arg->ToString(isolate->GetCurrentContext()));
        } else if (arg->IsArray()) {
            const v8::Local<v8::Array> array = arg.As<v8::Array>();

            const float x = utils::V8ValueToFloat(isolate, array->Get(isolate->GetCurrentContext(), 0).ToLocalChecked());
            const float y = utils::V8ValueToFloat(isolate, array->Get(isolate->GetCurrentContext(), 1).ToLocalChecked());
            const float z = utils::V8ValueToFloat(isolate, array->Get(isolate->GetCurrentContext(), 2).ToLocalChecked());

            PushArgumentAsType(invoker, NVector32(x, 0, y, 0, z, 0));
        }
    }

    ArgPtr PushArgPtr(CNativeInvoker* invoker, const CNativeValueType type) {
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

    v8::Local<v8::Value> PushValue(v8::Isolate* isolate, CNativeInvoker* invoker, CNativeValueType type) {
        auto lookupTable = Runtime::GetInstance()->GetLookupTable();

        switch (type) {
            case BOOL_TYPE: {
                return v8::Boolean::New(isolate, lookupTable->nativesFactory->GetReturnValueBuffer(invoker));
            }
            case FLOAT_TYPE: {
                return v8::Number::New(isolate, *static_cast<float*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker)));
            }
            case INT_TYPE: {
                return v8::Integer::New(isolate, *static_cast<int*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker)));
            }
            case UINT_TYPE: {
                return v8::Uint32::New(isolate, *static_cast<int32_t*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker)));
            }
            case VECTOR_TYPE: {
                NVector32 vector = *static_cast<NVector32*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker));

                const v8::Local<v8::Array> buffer = v8::Array::New(isolate,6);

                buffer->Set(isolate->GetCurrentContext(), 0, v8::Number::New(isolate, vector.x));
                buffer->Set(isolate->GetCurrentContext(), 1, v8::Number::New(isolate, vector.__padding_x));
                buffer->Set(isolate->GetCurrentContext(), 2, v8::Number::New(isolate, vector.y));
                buffer->Set(isolate->GetCurrentContext(), 3, v8::Number::New(isolate, vector.__padding_y));
                buffer->Set(isolate->GetCurrentContext(), 4, v8::Number::New(isolate, vector.z));
                buffer->Set(isolate->GetCurrentContext(), 5, v8::Number::New(isolate, vector.__padding_z));

                return buffer;
            }
            case STRING_TYPE: {
                return utils::StringToV8(isolate, *static_cast<std::string*>(lookupTable->nativesFactory->GetReturnValueBuffer(invoker)));
            }
            default: return {};
        }
    }

    v8::Local<v8::Value> PushValuePtr(v8::Isolate* isolate, CNativeInvoker* invoker, CNativeValueType type, void* buffer) {
        v8::Local<v8::Value> returnValue;

        switch (type) {
            case BOOL_TYPE: {
                returnValue = v8::Boolean::New(isolate, buffer);
                break;
            }
            case FLOAT_TYPE: {
                returnValue = v8::Number::New(isolate, *static_cast<float*>(buffer));
                break;
            }
            case INT_TYPE: {
                returnValue = v8::Integer::New(isolate, *static_cast<int*>(buffer));
                break;
            }
            case UINT_TYPE: {
                returnValue = v8::Uint32::New(isolate, *static_cast<int32_t*>(buffer));
                break;
            }
            case VECTOR_TYPE: {
                returnValue = v8::Boolean::New(isolate, buffer);
                break;
            }
            case STRING_TYPE: {
                const NVector32 vector = *static_cast<NVector32*>(buffer);

                const v8::Local<v8::Array> arrayBuffer = v8::Array::New(isolate,6);

                arrayBuffer->Set(isolate->GetCurrentContext(), 0, v8::Number::New(isolate, vector.x));
                arrayBuffer->Set(isolate->GetCurrentContext(), 1, v8::Number::New(isolate, vector.__padding_x));
                arrayBuffer->Set(isolate->GetCurrentContext(), 2, v8::Number::New(isolate, vector.y));
                arrayBuffer->Set(isolate->GetCurrentContext(), 3, v8::Number::New(isolate, vector.__padding_y));
                arrayBuffer->Set(isolate->GetCurrentContext(), 4, v8::Number::New(isolate, vector.z));
                arrayBuffer->Set(isolate->GetCurrentContext(), 5, v8::Number::New(isolate, vector.__padding_z));

                returnValue = arrayBuffer;
                break;
            }
            default: break;
        }

        return returnValue;
    }

    class NativeClass final {
    public:
        static std::string Name() {
            return "native";
        }

        static v8::Local<v8::ObjectTemplate> CreateTemplate(v8::Isolate* isolate) {
            v8::EscapableHandleScope escapableHandleScope(isolate);
            const v8::Local<v8::ObjectTemplate> oTemplate = v8::ObjectTemplate::New(isolate);

            oTemplate->SetInternalFieldCount(1);

            auto lookupTable = Runtime::GetInstance()->GetLookupTable();

            auto nativeRef = lookupTable->nativesFactory->CreateReflection();
            int32_t nativeSize;
            uint64_t* nativeData = nullptr;

            lookupTable->nativesFactory->GetListOfNatives(nativeRef, &nativeData, &nativeSize);

            for (int i = 0; i < nativeSize; i++) {
                const CNativeInformation* nativeInformation = lookupTable->nativesFactory->GetNativeInformation(nativeRef, nativeData[i]);
                if (nativeInformation == nullptr) {
                    continue;
                }

                oTemplate->Set(
                    utils::StringToV8(isolate, nativeInformation->m_Name),
                    v8::FunctionTemplate::New(isolate, NativeCallback, v8::BigInt::New(isolate,nativeInformation->m_Hash))
                );
            }

            delete nativeData;
            return oTemplate;
        };

    private:
        static void NativeCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            v8::Isolate* isolate = args.GetIsolate();
            v8::HandleScope handleScope(isolate);

            const auto lookupTable = Runtime::GetInstance()->GetLookupTable();
            const auto nativeRef = lookupTable->nativesFactory->CreateReflection();

            const CNativeInformation* nativeInformation = lookupTable->nativesFactory->GetNativeInformation(nativeRef, v8::Local<v8::BigInt>::Cast(args[0])->Uint64Value());

            if (nativeInformation == nullptr) {
                return;
            }

            if (CNativeInvoker* invoker = lookupTable->nativesFactory->CreateInvoker()) {
                std::vector<ArgPtr> returnPtrs;
                lookupTable->nativesFactory->Begin(invoker, nativeInformation->m_Hash);

                int skippingArgPtr = 0;

                for (int i = 0; i <= args.Length(); i++) {
                    CNativeValueInformation nativeParameter = nativeInformation->m_ParameterValueArrayData[i];
                    if (!nativeParameter.m_IsPointer) {
                        PushArg(isolate, args[i], invoker, nativeParameter.m_Type);
                    }
                    else {
                        PushArgPtr(invoker, nativeParameter.m_Type);
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

                const v8::Local<v8::Array> buffer = v8::Array::New(isolate,returnPtrs.size() + 2);

                buffer->Set(isolate->GetCurrentContext(), 0, PushValue(isolate, invoker, nativeInformation->m_ReturnValue.m_Type));

                for (int i = 0; i < returnPtrs.size(); i++) {
                    ArgPtr ret = returnPtrs[i];
                    buffer->Set(isolate->GetCurrentContext(), i + 1, PushValuePtr(isolate, invoker, ret.m_Type, ret.m_Ptr));

                    delete ret.m_Ptr;
                }

                const int defaultReturn = nativeInformation->m_ReturnValue.m_Type != VOID_TYPE;
                buffer->Set(isolate->GetCurrentContext(), returnPtrs.size() + 1, v8::Number::New(isolate, defaultReturn + returnPtrs.size()));
                args.GetReturnValue().Set(buffer);

                return;
            }

            args.GetReturnValue().Set(0);
        }
    };
}

#endif //V8_NATIVE_CLASS_H