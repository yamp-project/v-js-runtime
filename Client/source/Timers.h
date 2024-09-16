#pragma once

#include <fw/utils/InstanceBase.h>
#include <v8helper.h>

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

        STRONG_INLINE bool Repeat()
        {
            return type == Type::INTERVAL;
        }
    };

    class TimerFactory : public fw::utils::InstanceBase
    {
    public:
        static void CreateIntervalTimer(v8helper::FunctionContext& ctx);
        static void CreateTimeoutTimer(v8helper::FunctionContext& ctx);

        IMPLEMENT_INSTANCE_FUNCTION(TimerFactory);

    private:
        void CreateTimer(v8helper::FunctionContext& ctx, Timer::Type type);
    };

} // namespace js
