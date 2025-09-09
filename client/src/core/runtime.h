#ifndef RUNTIME_H
#define RUNTIME_H

#include <yamp-sdk/sdk.h>

#include <memory>
#include <optional>
#include <unordered_map>

#include "resource.h"
#include "util/logger.h"

namespace js
{
    bool Init();
    void Shutdown();
    void OnResourceStart(SDK_Resource* resource);
    void OnResourceStop(SDK_Resource* resource);
    void OnTick();
    void OnCoreEvent(CoreEventType type, CAnyArray* args);
    void OnResourceEvent(const char* name, CAnyArray* args);

    JSClassRef GetGlobalTemplate();

    class Runtime
    {
    public:
        static Runtime* GetInstance();
        static Runtime* Initialize(SDK_Interface* lookupTable);
        static void Shutdown();

        using Resources = std::unordered_map<SDK_Resource*, std::unique_ptr<Resource>>;

        explicit Runtime(SDK_Interface* lookupTable);
        ~Runtime() = default;

        SDK_Interface* GetLookupTable() const
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

        Resource* CreateResource(SDK_Resource* resource);
        Resource* GetResource(SDK_Resource* resource);

        void UnregisterResource(SDK_Resource *resource);

        std::optional<CoreEventType> GetCoreEventType(const char* eventName);

    private:
        static std::unique_ptr<Runtime> s_Instance;

        SDK_Interface* m_LookupTable = nullptr;
        Logger m_Logger;
        Resources m_Resources;

        std::unordered_map<std::string, CoreEventType> m_CoreEventMapping;
    };
}

#endif //RUNTIME_H