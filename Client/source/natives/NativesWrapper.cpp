#include "NativesWrapper.h"
#include "Runtime.h"

#include <v-sdk/factories/NativeFactory.hpp>

#define IMPLEMENT_PUSH_ARGUMENT(MappingIndex, Type)                                      \
    m_PushArgumentMapping[MappingIndex] = [](Ctx& ctx, Invoker* invoker, uint32_t index) \
    {                                                                                    \
        invoker->PushArgumentAsType(ctx.GetArg<Type>(index));                            \
    }

#define IMPLEMENT_RETURN_VALUE(MappingIndex, Type)                      \
    m_ReturnValueMapping[MappingIndex] = [](Ctx& ctx, Invoker* invoker) \
    {                                                                   \
        ctx.Return(invoker->GetReturnValueAs<Type>());                  \
    }

void NativesWrapper::InvokeNative(const v8::FunctionCallbackInfo<v8::Value>& _info)
{
    v8helper::FunctionContext ctx{_info};
    sdk::NativeInformation* native = static_cast<sdk::NativeInformation*>(_info.Data().As<v8::External>()->Value());
    if (ctx.GetArgCount() != native->m_ParameterValueArraySize)
    {
        js::Runtime::Log().Error("native call '{}' failed. Expected {} arguments but {} was passed.", native->m_Name, native->m_ParameterValueArraySize, ctx.GetArgCount());
        return;
    }

    static sdk::IInvokerFactory* invoker = sdk::IInvokerFactory::GetInstance();
    invoker->BeginCall();

    static NativesWrapper* wrapper = NativesWrapper::GetInstance();
    if (wrapper->TryToPushArguments(ctx, invoker, native))
    {
        invoker->EndCall(native->m_Hash);
        wrapper->TryToReturnValue(ctx, invoker, native->m_ReturnValue.m_Type);
    }
}

NativesWrapper::NativesWrapper() : m_PushArgumentMapping(), m_ReturnValueMapping()
{
    IMPLEMENT_PUSH_ARGUMENT(sdk::NativeValueType::Bool, bool);
    IMPLEMENT_PUSH_ARGUMENT(sdk::NativeValueType::Int, int32_t);
    IMPLEMENT_PUSH_ARGUMENT(sdk::NativeValueType::UnsignedInt, uint32_t);
    IMPLEMENT_PUSH_ARGUMENT(sdk::NativeValueType::Float, float);
    IMPLEMENT_PUSH_ARGUMENT(sdk::NativeValueType::String, const char*);

    IMPLEMENT_RETURN_VALUE(sdk::NativeValueType::Bool, bool);
    IMPLEMENT_RETURN_VALUE(sdk::NativeValueType::Int, int32_t);
    IMPLEMENT_RETURN_VALUE(sdk::NativeValueType::UnsignedInt, uint32_t);
    IMPLEMENT_RETURN_VALUE(sdk::NativeValueType::Float, float);
    IMPLEMENT_RETURN_VALUE(sdk::NativeValueType::String, std::string);
}

bool NativesWrapper::TryToPushArguments(Ctx& ctx, Invoker* invoker, sdk::NativeInformation* native)
{
    for (uint32_t i = 0; i < native->m_ParameterValueArraySize; ++i)
    {
        auto handler = m_PushArgumentMapping[native->m_ParameterValueArrayData[i].m_Type];
        if (handler == nullptr)
        {
            // TODO: log the current resource name
            js::Runtime::Log().Error("unsupported data type {} for the native {} at the index {}", (uint8_t)native->m_ParameterValueArrayData[i].m_Type, native->m_Name, i);
            return false;
        }

        handler(ctx, invoker, i);
    }

    return true;
}

void NativesWrapper::TryToReturnValue(Ctx& ctx, Invoker* invoker, uint8_t type)
{
    if (auto handler = m_ReturnValueMapping[type])
    {
        handler(ctx, invoker);
    }
}
