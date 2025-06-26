#include "script_runtime.h"

#include <string>

namespace js
{
    bool Init()
    {
        ScriptRuntime::GetInstance()->GetLogger()->Debug("ScriptRuntime::Init()");
        return true;
    }

    void Shutdown()
    {
        ScriptRuntime::GetInstance()->GetLogger()->Debug("ScriptRuntime::BeforeShutdown()");
    }

    void OnResourceStart(IResource* resource)
    {
        ScriptRuntime::GetInstance()->GetLogger()->Debug(("ScriptRuntime::OnResourceStart() - Resource: " + std::string(resource->GetName())).c_str());
    }

    void OnResourceStop(IResource* resource)
    {
        ScriptRuntime::GetInstance()->GetLogger()->Debug("ScriptRuntime::BeforeShutdown()");
    }

    void OnTick()
    {
        ScriptRuntime::GetInstance()->GetLogger()->Debug("ScriptRuntime::OnTick()");
    }

    void OnEvent(void* event)
    {
        ScriptRuntime::GetInstance()->GetLogger()->Debug(("ScriptRuntime::OnEvent()"));
    }
}
