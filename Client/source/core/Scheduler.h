#pragma once

#include <v8helper.h>
#include <map>

using TimerExpiry = std::chrono::time_point<std::chrono::steady_clock, std::chrono::steady_clock::duration>;

namespace js
{
    struct Timer
    {
        enum Type : uint8_t
        {
            INTERVAL,
            TIMEOUT,
            IMMEDIATE
        };

        v8helper::Persistent<v8::Function> callback;
        int32_t delay;
        Type type;

        inline bool Repeat()
        {
            return type == Type::INTERVAL;
        }
    };

    class Resource;
    class Scheduler
    {
    public:
        static void CreateTimer(v8helper::FunctionContext& ctx, Timer::Type type);

        Scheduler(js::Resource* parentResource) : m_ParentResource(parentResource), m_Timers(), m_ExpiredTimers()
        {
            //
        }

        void RegisterTimer(TimerExpiry expiry, Timer&& timer);
        void ProcessTimers();

    private:
        js::Resource* m_ParentResource;

        std::multimap<std::chrono::time_point<std::chrono::steady_clock>, Timer> m_Timers; // A multimap to store timers by their expiry time (sorted automatically)
        std::vector<Timer> m_ExpiredTimers;                                                // Reused vector
    };
} // namespace js
