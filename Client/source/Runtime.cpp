#include "Runtime.h"
#include "Resource.h"
#include "helpers.h"

#include <v-sdk/factories/ResourceFactory.hpp>
#include <libplatform/libplatform.h>

namespace js
{
    Runtime::Runtime() : m_LoadedResources(), m_Isolate(nullptr)
    {
        //
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
        if (!std::string(resourceInformation->m_MainFile).ends_with(".js"))
        {
            Log().Error("The resource {} has no main file valid js file extension `{}`", resourceInformation->m_Name, resourceInformation->m_MainFile);
            return {false};
        }

        Resource* newResource = new Resource(resourceInformation, m_Isolate);
        sdk::Result resourceCreationResult = sdk::IResourceFactory::GetInstance()->RegisterResource(newResource);
        m_LoadedResources.push_back(newResource);

        Log().Info("Resource loaded: {} {}", resourceInformation->m_Path, resourceInformation->m_MainFile);
        return {true};
    }
} // namespace js
