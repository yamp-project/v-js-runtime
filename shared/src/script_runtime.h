#pragma once

#include <yamp-sdk/sdk.h>
#include <assert.h>

namespace yamp::js
{
    bool Init();
    void Shutdown();

    void OnResourceStart(IResource* resource);
    void OnResourceStop(IResource* resource);

    void OnTick();
    void OnEvent(void* event);

    class ScriptRuntime
    {
    public:
        static ScriptRuntime* GetInstance()
        {
            static ScriptRuntime instance;
            return &instance;
        }

        void SetupLookupTable(ILookupTable* lookupTable)
        {
            assert(m_LookupTable == nullptr);
            m_LookupTable = lookupTable;
        }

        ILogger* GetLogger()
        {
            assert(m_LookupTable != nullptr);
            return m_LookupTable->GetLogger();
        }

    private:
        ILookupTable* m_LookupTable = nullptr;
    };
}
