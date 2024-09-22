#include "NativeInvoker.h"

#include "Resource.h"
#include "Runtime.h"

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

#define DEBUG_BUILD

void NativeInvoker::Invoke(const v8::FunctionCallbackInfo<v8::Value>& _info)
{
    sdk::NativeInformation* native = static_cast<sdk::NativeInformation*>(_info.Data().As<v8::External>()->Value());
    v8helper::FunctionContext ctx{_info};

    js::Resource* resource = js::Runtime::GetInstance()->GetResourceByContext(ctx.GetContext());
    if (resource == nullptr)
    {
        js::Runtime::Log().Error("native call '{}' failed, no bound resource found.", native->m_Name);
        return;
    }

    if (ctx.GetArgCount() != native->m_ParameterValueArraySize)
    {
        js::Runtime::Log().Error("native call '{}' failed. Expected {} arguments but {} was passed.", native->m_Name, native->m_ParameterValueArraySize, ctx.GetArgCount());
        return;
    }

    sdk::IInvokerFactory* sdkInvoker = sdk::IInvokerFactory::GetInstance();
    sdkInvoker->BeginCall();

    NativeInvoker& invoker = resource->GetNativeInvoker();
    if (invoker.TryToPushArguments(ctx, sdkInvoker, native))
    {
        sdkInvoker->EndCall(native->m_Hash);
        invoker.TryToReturnValue(ctx, sdkInvoker, native->m_ReturnValue.m_Type);
    }
}

NativeInvoker::NativeInvoker(js::Resource* parentResource) : m_ParentResource(parentResource), m_PushArgumentMapping(), m_ReturnValueMapping()
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

bool NativeInvoker::TryToPushArguments(Ctx& ctx, Invoker* invoker, sdk::NativeInformation* native)
{
    for (uint32_t i = 0; i < native->m_ParameterValueArraySize; ++i)
    {
        auto handler = m_PushArgumentMapping[native->m_ParameterValueArrayData[i].m_Type];
#ifdef DEBUG_BUILD
        if (handler == nullptr)
        {
            // TODO: log the current resource name
            js::Runtime::Log().Error("unsupported data type {} for the native {} at the index {}", (uint8_t)native->m_ParameterValueArrayData[i].m_Type, native->m_Name, i);
            return false;
        }
#endif
        handler(ctx, invoker, i);
    }

    return true;
}

void NativeInvoker::TryToReturnValue(Ctx& ctx, Invoker* invoker, uint8_t type)
{
    if (auto handler = m_ReturnValueMapping[type])
    {
        handler(ctx, invoker);
    }
}
