#include "ResourceScheduler.h"

#include "Resource.h"
#include "stdafx.h"

namespace js
{
    void ResourceScheduler::RegisterTimer(TimerExpiry expiry, Timer&& timer)
    {
        m_Timers.emplace(expiry, std::move(timer));
    }

    void ResourceScheduler::ProcessTimers()
    {
        auto now = std::chrono::steady_clock::now();
        v8::Isolate* isolate = m_ParentResource->GetIsolate();
        V8_SCOPE(isolate);

        auto context = m_ParentResource->GetContext().Get(isolate);
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
} // namespace js
