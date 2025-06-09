
#include "ScriptRuntime.h"
#include "version.h"

#include <yamp-sdk/sdk.h>
#include <yamp-sdk/version.h>

void RuntimeEntry(RegisterRuntime registerRuntime)
{
    IScriptRuntime* runtime = registerRuntime("js");
    yamp::js::ScriptRuntime::SetScriptRuntimeInstance(runtime);
    runtime->GetLogger()->Debug("RuntimeEntry()");    
}

const char* GetSdkVersion()
{
    return YAMP_SDK_VERSION;
}

const char* GetModuleVersion()
{
    return YAMP_MODULE_VERSION;
}
