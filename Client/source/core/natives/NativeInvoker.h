#pragma once

#include "stdafx.h"

#include <v-sdk/factories/NativeFactory.hpp>
#include <v8helper.h>

using namespace yamp;

namespace js
{
    class Resource;
}

namespace core
{
    class NativeInvoker
    {
    private:
        using Ctx = v8helper::FunctionContext;
        using Invoker = sdk::IInvokerFactory;

    public:
        static void Invoke(const v8::FunctionCallbackInfo<v8::Value>& _info);

        NativeInvoker(js::Resource* parentResource);

        STRONG_INLINE bool TryToPushArguments(Ctx& ctx, Invoker* invoker, sdk::NativeInformation* native);
        STRONG_INLINE void TryToReturnValue(Ctx& ctx, Invoker* invoker, uint8_t type);

    private:
        js::Resource* m_ParentResource;

        // TODO: replace 7 with enum max value
        void (*m_PushArgumentMapping[7])(Ctx& ctx, Invoker* invoker, uint32_t index);
        void (*m_ReturnValueMapping[7])(Ctx& ctx, Invoker* invoker);
    };
} // namespace core
