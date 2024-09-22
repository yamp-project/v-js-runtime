#include "globals.h"

#include "core/Scheduler.h"

#include <fw/Logger.h>

namespace bindings::global
{
    void SetInterval(v8helper::FunctionContext& ctx)
    {
        js::Scheduler::CreateTimer(ctx, js::Timer::Type::INTERVAL);
    }

    void SetTimeout(v8helper::FunctionContext& ctx)
    {
        js::Scheduler::CreateTimer(ctx, js::Timer::Type::TIMEOUT);
    }

    void Print(v8helper::FunctionContext& ctx)
    {
        if (ctx.GetArgCount() == 0)
        {
            return;
        }

        std::stringstream buffer;
        for (int i = 0, argsCount = ctx.GetArgCount(); i < argsCount; i++)
        {
            v8::Local<v8::Value> val = ctx.GetArg<v8::Local<v8::Value>>(i);
            if (!val.IsEmpty())
            {
                buffer << v8helper::Stringify(val);

                if (i != argsCount - 1)
                {
                    buffer << " ";
                }
            }
        }

        std::string message = buffer.str();
        if (message.empty())
        {
            fw::Logger::Get("JS")->Warn("{}", "April-snow-flake: could not print");
        }
        else
        {
            fw::Logger::Get("JS")->Info("{}", message);
        }
    }
} // namespace bindings::global
