#include <yamp-sdk/sdk.h>
#include "core/runtime.h"

SDK_Context GetRuntimeContext()
{
    return {
        .version = "0.0.1",
        .sdkVersion = "0.0.1",

        .Init = js::Init,
        .Shutdown = js::Shutdown,

        .OnResourceStart = js::OnResourceStart,
        .OnResourceStop = js::OnResourceStop,
        .OnTick = js::OnTick,
        .OnCoreEvent = js::OnCoreEvent,
        .OnResourceEvent = js::OnResourceEvent,
    };
}

SDK_EXPORT void RuntimeEntry(const RegisterRuntime registerRuntime)
{
    js::Runtime* runtime = js::Runtime::Initialize(registerRuntime("js", GetRuntimeContext()));
    runtime->GetLogger().Info("Javascript runtime registered! 👍");
}