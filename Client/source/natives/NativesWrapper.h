#pragma once

#include "fw/utils/InstanceBase.h"
#include "stdafx.h"

#include <v-sdk/factories/NativeFactory.hpp>
#include <v8helper.h>

using namespace yamp;

class NativesWrapper : public fw::utils::InstanceBase
{
private:
    using Ctx = v8helper::FunctionContext;
    using Invoker = sdk::IInvokerFactory;

public:
    static void InvokeNative(const v8::FunctionCallbackInfo<v8::Value>& _info);

    NativesWrapper();

    STRONG_INLINE bool TryToPushArguments(Ctx& ctx, Invoker* invoker, sdk::NativeInformation* native);
    STRONG_INLINE void TryToReturnValue(Ctx& ctx, Invoker* invoker, uint8_t type);

    IMPLEMENT_INSTANCE_FUNCTION(NativesWrapper);

private:
    // TODO: replace 7 with enum max value
    void (*m_PushArgumentMapping[7])(Ctx& ctx, Invoker* invoker, uint32_t index);
    void (*m_ReturnValueMapping[7])(Ctx& ctx, Invoker* invoker);
};
