#include "Timers.h"

#include "Scheduler.h"
#include "Resource.h"
#include "Runtime.h"

#include <fw/Logger.h>

namespace js
{
    void TimerFactory::CreateIntervalTimer(v8helper::FunctionContext& ctx)
    {
        TimerFactory::GetInstance()->CreateTimer(ctx, Timer::Type::INTERVAL);
    }

    void TimerFactory::CreateTimeoutTimer(v8helper::FunctionContext& ctx)
    {
        TimerFactory::GetInstance()->CreateTimer(ctx, Timer::Type::TIMEOUT);
    }

    void TimerFactory::CreateTimer(v8helper::FunctionContext& ctx, Timer::Type type)
    {
        Resource* resource = Runtime::GetInstance()->GetResourceByContext(ctx.GetContext());
        if (!ctx.CheckArgCount(2) || resource == nullptr)
        {
            return;
        }

        v8::Isolate* isolate = ctx.GetIsolate();
        v8::HandleScope scope(isolate);

        v8::Local<v8::Value> callbackValue;
        if (!ctx.GetArg(0, callbackValue, v8helper::Type::FUNCTION))
        {
            return;
        }

        auto callback = v8helper::Persistent<v8::Function>(isolate, v8::Local<v8::Function>::Cast(callbackValue));
        int32_t delay = ctx.GetArg<int32_t>(1);

        TimerExpiry expiry = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay);
        resource->GetScheduler().RegisterTimer(expiry, {callback, delay, type});
    }
} // namespace js
