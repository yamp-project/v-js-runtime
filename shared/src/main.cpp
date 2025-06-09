
#include "ScriptRuntime.h"
#include "version.h"

#include <yamp-sdk/sdk.h>
#include <yamp-sdk/version.h>

void RuntimeEntry(RegisterRuntime* registerRuntimeFn)
{
    IScriptRuntime runtime;
    if (!registerRuntimeFn("js", &runtime))
        return;

    yamp::js::ScriptRuntime::SetScriptRuntimeInstance(runtime);
    runtime.GetLogger()->Debug("ModuleInit()");    
}

const char* GetSdkVersion()
{
    return YAMP_SDK_VERSION;
}

const char* GetModuleVersion()
{
    return YAMP_MODULE_VERSION;
}
