#include "Scheduler.h"

#include "Resource.h"
#include "Runtime.h"
#include "stdafx.h"

#include <fw/Logger.h>

namespace core
{
    void Scheduler::CreateTimer(v8helper::FunctionContext& ctx, Timer::Type type)
    {
        js::Resource* resource = js::Runtime::GetInstance()->GetResourceByContext(ctx.GetContext());
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

    Scheduler::Scheduler(js::Resource* parentResource) : m_ParentResource(parentResource), m_Timers(), m_ExpiredTimers()
    {
        //
    }

    void Scheduler::RegisterTimer(TimerExpiry expiry, Timer&& timer)
    {
        m_Timers.emplace(expiry, std::move(timer));
    }

    void Scheduler::ProcessTimers()
    {
        auto now = std::chrono::steady_clock::now();
        v8::Isolate* isolate = m_ParentResource->GetIsolate();

        v8::Local<v8::Context> context = m_ParentResource->GetContext();
        auto hint = m_Timers.end();
        m_ExpiredTimers.clear();

        // Collect all expired timers, this reduces multimap tree rebalance
        auto it = m_Timers.begin();
        while (it != m_Timers.end() && it->first <= now)
        {
            m_ExpiredTimers.push_back(std::move(it->second));
            it = m_Timers.erase(it);
        }

        // Process expired timers
        for (Timer& timer : m_ExpiredTimers)
        {
            v8::Local<v8::Function> callback = timer.callback.Get(isolate);
            bool hasErrored = false;

            if (callback.IsEmpty() || !callback->IsFunction())
            {
                fw::Logger::Get("DEBUG")->Error("Error: Callback is invalid");
                hasErrored = true;
            }
            else
            {
                v8::TryCatch tryCatch(isolate);
                auto result = callback->Call(context, context->Global(), 0, nullptr);

                if (tryCatch.HasCaught())
                {
                    hasErrored = true;
                    v8::String::Utf8Value error(isolate, tryCatch.Exception());
                    fw::Logger::Get("DEBUG")->Error("Error during callback execution: {}", std::string(*error));
                }
            }

            if (!hasErrored)
            {
                // Re-add the timer if it should repeat
                if (timer.Repeat())
                {
                    hint = m_Timers.emplace_hint(hint, now + std::chrono::milliseconds(timer.delay), std::move(timer));
                }

                // Update 'now' after each callback execution
                now = std::chrono::steady_clock::now();
            }
        }
    }
} // namespace core
