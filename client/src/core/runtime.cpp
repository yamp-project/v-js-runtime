#include "runtime.h"
#include "util/logger.h"
#include "util/utils.h"

#include <cassert>
#include <ranges>
#include <utility>

namespace js
{
    bool Init()
    {
        Runtime* runtime = Runtime::GetInstance();

        runtime->GetLogger().Info("Javascript runtime initializing");

        return true;
    }

    void OnResourceStart(SDK_Resource* resource)
    {
        Runtime* runtime = Runtime::GetInstance();
        if (Resource* tlResource = runtime->GetResource(resource); tlResource)
        {
            tlResource->OnStart();
            return;
        }

        Resource* tlResource = runtime->CreateResource(resource);
        tlResource->OnStart();

        runtime->GetLogger().Info("Resource %s started", resource->name);
    }

    void OnResourceStop(SDK_Resource* resource)
    {
        Runtime::GetInstance()->GetLogger().Info("Resource %s stopped", resource->name);
        Runtime::GetInstance()->GetResource(resource)->OnStop();
        Runtime::GetInstance()->UnregisterResource(resource);
    }

    void OnTick()
    {
        //
    }

    void OnCoreEvent(CoreEventType type, CAnyArray* args)
    {
        Runtime* runtime = Runtime::GetInstance();
        runtime->GetLogger().Info("Event triggered %s", type);

        for (const auto& val : runtime->GetResources() | std::views::values)
        {
            val->OnEvent(type, args);
        }
    }

    void OnResourceEvent(const char* name, CAnyArray* args)
    {
        Runtime* runtime = Runtime::GetInstance();
        runtime->GetLogger().Info("Event triggered %s", name);

        for (const auto& val : runtime->GetResources() | std::views::values)
        {
            val->OnEvent(name, args);
        }
    }

    JSClassRef GetGlobalTemplate() {
    }

    std::unique_ptr<Runtime> Runtime::s_Instance = nullptr;

    Runtime* Runtime::GetInstance()
    {
        assert(s_Instance != nullptr);
        return s_Instance.get();
    }

    Runtime* Runtime::Initialize(SDK_Interface* lookupTable)
    {
        assert(s_Instance == nullptr);
        s_Instance = std::make_unique<Runtime>(lookupTable);

        CoreEventMetas eventMetas = s_Instance->GetLookupTable()->GetCoreEventMetas();
        for (size_t i = 0; i < eventMetas.size; ++i)
        {
            CoreEventMeta& eventMeta = eventMetas.buffer[i];
            s_Instance->m_CoreEventMapping[::utils::StrToCamelCase(eventMeta.name)] = eventMeta.type;
        }

        return s_Instance.get();
    }

    void Runtime::Shutdown()
    {
        assert(s_Instance != nullptr);
        s_Instance.reset();
    }

    Runtime::Runtime(SDK_Interface* lookupTable) : m_LookupTable(lookupTable), m_Logger(Logger(lookupTable, "js"))
    {
    }

    Resource* Runtime::GetResource(SDK_Resource* resource)
    {
        auto it = m_Resources.find(resource);
        if (it != m_Resources.end())
        {
            return it->second.get();
        }

        return nullptr;
    }

    void Runtime::UnregisterResource(SDK_Resource* resource) {
        const auto it = m_Resources.find(resource);
        if (it == m_Resources.end())
        {
            return;
        }

        it->second.reset();

        m_Resources.erase(resource);
    }

    Resource* Runtime::CreateResource(SDK_Resource* sdk_resource)
    {
        auto resourcePtr = std::make_unique<Resource>(m_LookupTable, sdk_resource);

        m_Resources[sdk_resource] = std::move(resourcePtr);

        return m_Resources[sdk_resource].get();
    }

    std::optional<CoreEventType> Runtime::GetCoreEventType(const char* eventName)
    {
        const auto it = m_CoreEventMapping.find(eventName);
        if (it != m_CoreEventMapping.end()) {
            return std::optional{it->second};
        }

        return std::nullopt;
    }
}
