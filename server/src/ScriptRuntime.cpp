#include "ScriptRuntime.h"

#include <string>

namespace yamp::js
{
    bool ScriptRuntime::Init()
    {
        GetLogger()->Debug("ScriptRuntime::Init()");
        return true;
    }

    void ScriptRuntime::BeforeShutdown()
    {
        GetLogger()->Debug("ScriptRuntime::BeforeShutdown()");
    }

    void ScriptRuntime::OnResourceStart(yamp::sdk::IResource* resource)
    {
        GetLogger()->Debug(("ScriptRuntime::OnResourceStart() - Resource: " + std::string(resource->GetName())).c_str());
    }

    void ScriptRuntime::OnResourceStop(yamp::sdk::IResource* resource)
    {
        GetLogger()->Debug("ScriptRuntime::BeforeShutdown()");
    }

    void ScriptRuntime::OnTick()
    {
        GetLogger()->Debug("ScriptRuntime::OnTick()");
    }

    void ScriptRuntime::OnEvent(yamp::sdk::IEventBase* event)
    {
        GetLogger()->Debug(("ScriptRuntime::OnEvent() - Event Type: " + std::to_string(static_cast<int>(event->GetEventType()))).c_str());
    }
}
