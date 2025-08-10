#ifndef RUNTIME_H
#define RUNTIME_H

#include <yamp-sdk/sdk.h>

#include <memory>
#include <unordered_map>

#include "resource.h"
#include "util/logger.h"

namespace js
{
    bool Init();
    void Shutdown();
    void OnResourceStart(IResource* resource);
    void OnResourceStop(IResource* resource);
    void OnTick();
    void OnCoreEvent(CoreEventType type, CAnyArray* args);
    void OnResourceEvent(const char* name, CAnyArray* args);

    void ShutdownV8();

    class Runtime
    {
    public:
        static Runtime* GetInstance();
        static Runtime* Initialize(ILookupTable* lookupTable);
        static void Shutdown();

        using Resources = std::unordered_map<IResource*, std::unique_ptr<Resource>>;

        Runtime(ILookupTable* lookupTable);
        ~Runtime() = default;

        ILookupTable* GetLookupTable() const
        {
            return m_LookupTable;
        }

        Resources& GetResources()
        {
            return m_Resources;
        }

        Logger& GetLogger()
        {
            return m_Logger;
        }

        std::vector<std::unique_ptr<v8::Isolate>>& GetIsolates()
        {
            return m_Isolates;
        }

        Resource* CreateResource(IResource* resource);
        Resource* GetResource(IResource* resource);

        std::optional<CoreEventType> GetCoreEventType(const char* eventName);

    private:
        static std::unique_ptr<Runtime> s_Instance;

        ILookupTable* m_LookupTable = nullptr;
        Logger m_Logger;
        Resources m_Resources;

        // V8
        v8::Isolate::CreateParams m_IsolateParams;
        std::vector<std::unique_ptr<v8::Isolate>> m_Isolates;

        std::unordered_map<std::string, CoreEventType> m_CoreEventMapping;
    };
}

#endif //RUNTIME_H