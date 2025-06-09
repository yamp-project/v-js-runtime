#pragma once

#include <yamp-sdk/sdk.h>

namespace yamp::js
{
    class ScriptRuntime
    {
    public:
        static void SetScriptRuntimeInstance(IScriptRuntime* runtime)
        {
            m_ScriptRuntime = runtime;            
        }

        static ScriptRuntime* GetInstance()
        {
            static ScriptRuntime instance;
            return &instance;
        }
        
    private:
        static IScriptRuntime* m_ScriptRuntime;
    };
}
