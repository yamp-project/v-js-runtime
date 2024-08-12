#include "Runtime.h"
#include "Resource.h"
#include "helpers.h"
#include "events/EventManager.h"

#include <libplatform/libplatform.h>
#include <v-sdk/factories/ResourceFactory.hpp>
#include <v-sdk/Result.hpp>
#include <Windows.h>

namespace js
{
    void Runtime::OnEvent(const char* eventName, PolymorphicalValue* args[], size_t size)
    {
        auto logger = fw::Logger::Get("rt-debug");
        logger->Info("RECEIVED EVENT {} with {} arguments", eventName, size);

        for (auto resource : js::Runtime::GetInstance()->GetResources())
        {
            resource->GetEventManager()->Fire(eventName);
        }

        // for (size_t i = 0; i < size; ++i)
        // {
        //     PolymorphicalValue* arg = args[i];

        //     switch (arg->m_Type)
        //     {
        //     case PolymorphicalValueType::NUMBER:
        //         logger->Info("argument {} is of type number and its value is: {}", i, arg->As<double>());
        //         break;

        //     case PolymorphicalValueType::BOOLEAN:
        //         logger->Info("argument {} is of type boolean and its value is: {}", i, arg->As<bool>());
        //         break;

        //     case PolymorphicalValueType::STRING:
        //         logger->Info("argument {} is of type string and its value is: {}", i, arg->As<const char*>());
        //         break;

        //     default:
        //         break;
        //     }
        // }
    }

    Runtime::Runtime() : m_Resources(), m_Isolate(nullptr)
    {
    }

    Runtime::~Runtime()
    {
        //
    }

    void Runtime::SetupIsolate()
    {
        v8::V8::SetFlagsFromString("--harmony-import-assertions --short-builtin-calls --no-lazy --no-flush-bytecode");

        m_Platform = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(m_Platform.get());
        v8::V8::InitializeICU("D:/.yamp/v-client/bin/runtimes/icudtl_v8.dat");
        v8::V8::Initialize();

        v8::Isolate::CreateParams createParams;
        createParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        createParams.allow_atomics_wait = false;

        m_Isolate = v8::Isolate::New(createParams);
        Assert(m_Isolate != nullptr, "Failed to create isolate");

        m_Isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
        m_Isolate->SetCaptureStackTraceForUncaughtExceptions(true, 5);
    }

    void Runtime::OnStart()
    {
        SetupIsolate();

        Log().Info("Succesfully started");

        // TODO: move to the sdk
        HMODULE clientModule = GetModuleHandleA("yamp_client.dll");
        auto RegisterOnEventCallback = reinterpret_cast<void (*)(void(const char*, PolymorphicalValue**, size_t))>(GetProcAddress(clientModule, "OnEvent"));
        RegisterOnEventCallback(Runtime::OnEvent);
    }

    void Runtime::OnStop()
    {
        v8helper::ClassInstanceCache::Clear(m_Isolate);
        v8helper::Namespace::Cleanup(m_Isolate);
        v8helper::Module::Cleanup(m_Isolate);
        v8helper::Class::Cleanup(m_Isolate);

        delete m_Isolate->GetArrayBufferAllocator();

        v8::V8::Dispose();
        v8::V8::DisposePlatform();
    }

    void Runtime::OnTick()
    {
        v8::Locker locker(m_Isolate);
        v8::Isolate::Scope isolateScope(m_Isolate);
        v8::HandleScope handleScope(m_Isolate);

        v8::platform::PumpMessageLoop(m_Platform.get(), m_Isolate);
        Log().Info("tick..");
    }

    sdk::Result Runtime::OnHandleResourceLoad(sdk::ResourceInformation* resourceInformation)
    {
        std::string mainFile = resourceInformation->m_MainFile;
        bool isTypescript = mainFile.ends_with(".ts");
        if (!mainFile.ends_with(".js") && !isTypescript)
        {
            Log().Error("The resource {} has no main file valid js/ts extension `{}`", resourceInformation->m_Name, resourceInformation->m_MainFile);
            return {false};
        }

        Resource* resource = new Resource(m_Isolate, resourceInformation, isTypescript);
        sdk::Result result = sdk::IResourceFactory::GetInstance()->RegisterResource(resource);

        // TODO(YANN): debug this
        // if (!result.WasSuccessful())
        // {
        //     delete resource;
        //     return result;
        // }

        m_Resources.push_back(resource);

        Log().Info("Resource loaded: {} {}", resourceInformation->m_Path, resourceInformation->m_MainFile);
        return {true};
    }

    Resource* Runtime::GetResourceByContext(v8::Local<v8::Context> context) const
    {
        for (auto resource : m_Resources)
        {
            if (resource->GetContext().Get(m_Isolate) == context)
            {
                return resource;
            }
        }

        return nullptr;
    }
} // namespace js
