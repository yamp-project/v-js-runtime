
#include "script_runtime.h"
#include "version.h"

#include <yamp-sdk/version.h>
#include <yamp-sdk/sdk.h>

SDK_EXPORT void RuntimeEntry(RegisterRuntime registerRuntime)
{
    yamp::js::ScriptRuntime* runtime = yamp::js::ScriptRuntime::GetInstance();

    runtime->SetupLookupTable(registerRuntime("js", {
        .sdkVersion = YAMP_SDK_VERSION,
        .version = YAMP_RUNTIME_VERSION,

        .Init = yamp::js::Init,
        .BeforeShutdown = yamp::js::BeforeShutdown,
        .OnResourceStart = yamp::js::OnResourceStart,
        .OnResourceStop = yamp::js::OnResourceStop,

        .OnTick = yamp::js::OnTick,
        .OnEvent = yamp::js::OnEvent
    }));

    runtime->GetLogger()->Info("hello there from JS !\n");
}
