#pragma once

#include "Timers.h"

#include <map>

using TimerExpiry = std::chrono::time_point<std::chrono::steady_clock, std::chrono::steady_clock::duration>;

namespace js
{
    class Resource;
    class ResourceScheduler
    {
    public:
        ResourceScheduler(js::Resource* parentResource) : m_ParentResource(parentResource), m_Timers(), m_ExpiredTimers()
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
