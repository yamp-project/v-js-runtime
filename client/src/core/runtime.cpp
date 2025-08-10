#include "runtime.h"
#include "util/logger.h"
#include "util/utils.h"

#include <cassert>
#include <utility>
#include <v8.h>
#include <v8/include/libplatform/libplatform.h>

namespace js
{
    bool Init()
    {
        Runtime* runtime = Runtime::GetInstance();

        runtime->GetLogger().Info("Javascript runtime initializing");

        // Init V8
        v8::V8::InitializeICUDefaultLocation("");
        v8::V8::InitializeExternalStartupData("");
        std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform.get());
        v8::V8::Initialize();

        return true;
    }

    void Shutdown()
    {
        ShutdownV8();
    }

    void OnResourceStart(IResource* resource)
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

    void OnResourceStop(IResource* resource)
    {
        Runtime::GetInstance()->GetLogger().Info("Resource %s stopped", resource->name);
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

    void ShutdownV8()
    {
        for (auto& isolate : Runtime::GetInstance()->GetIsolates())
        {
            isolate->Dispose();
            isolate.release();
        }
        Runtime::GetInstance()->GetIsolates().clear();

        v8::V8::Dispose();
        v8::V8::DisposePlatform();
    }

    std::unique_ptr<Runtime> Runtime::s_Instance = nullptr;

    Runtime* Runtime::GetInstance()
    {
        assert(s_Instance != nullptr);
        return s_Instance.get();
    }

    Runtime* Runtime::Initialize(ILookupTable* lookupTable)
    {
        assert(s_Instance == nullptr);
        s_Instance = std::make_unique<Runtime>(lookupTable);

        CoreEventMetas eventMetas = s_Instance->GetLookupTable()->GetCoreEventMetas();
        for (size_t i = 0; i < eventMetas.size; ++i)
        {
            CoreEventMeta& eventMeta = eventMetas.buffer[i];
            s_Instance->m_CoreEventMapping[::utils::StrToCamelCase(eventMeta.name)] = eventMeta.type;
        }

        s_Instance->m_IsolateParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

        return s_Instance.get();
    }

    void Runtime::Shutdown()
    {
        assert(s_Instance != nullptr);
        s_Instance.reset();
    }

    Runtime::Runtime(ILookupTable* lookupTable) : m_LookupTable(lookupTable), m_Logger(Logger(lookupTable, "js"))
    {
    }

    Resource* Runtime::GetResource(IResource* resource)
    {
        auto it = m_Resources.find(resource);
        if (it != m_Resources.end())
        {
            return it->second.get();
        }

        return nullptr;
    }

    Resource* Runtime::CreateResource(IResource* iResource)
    {
        std::unique_ptr<v8::Isolate> isolate(v8::Isolate::New(m_IsolateParams));

        v8::Isolate* isolatePtr = isolate.get();

        m_Isolates.push_back(std::move(isolate));

        auto resourcePtr = std::make_unique<Resource>(m_LookupTable, iResource, isolatePtr);

        m_Resources[iResource] = std::move(resourcePtr);

        return m_Resources[iResource].get();
    }

    std::optional<CoreEventType> Runtime::GetCoreEventType(const char* eventName)
    {
        auto it = m_CoreEventMapping.find(eventName);
        if (it != m_CoreEventMapping.end()) {
            return std::optional{it->second};
        }

        return std::nullopt;
    }
}
