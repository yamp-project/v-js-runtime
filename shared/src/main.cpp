
#include "script_runtime.h"
#include "version.h"

#include <yamp-sdk/version.h>
#include <yamp-sdk/sdk.h>

IRuntimeContext GetRuntimeContext()
{
    return {
        .version    = YAMP_RUNTIME_VERSION,
        .sdkVersion = YAMP_SDK_VERSION,

        // lifecycle
        .Init     = js::Init,
        .Shutdown = js::Shutdown,

        // events
        .OnResourceStart = js::OnResourceStart,
        .OnResourceStop  = js::OnResourceStop,
        .OnTick          = js::OnTick,
        .OnEvent         = js::OnEvent
    };
}

SDK_EXPORT void RuntimeEntry(RegisterRuntime registerRuntime)
{
    js::ScriptRuntime* runtime = js::ScriptRuntime::GetInstance();
    runtime->SetupLookupTable(registerRuntime("js", GetRuntimeContext()));
    runtime->GetLogger()->Info("hello there from JS !\n");
}
