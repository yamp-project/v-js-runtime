#include "Runtime.h"
#include "Resource.h"
#include "helpers.h"

#include <v-sdk/factories/ResourceFactory.hpp>
#include <libplatform/libplatform.h>
#include <fw/Logger.h>
namespace js
{
    Runtime::Runtime() : m_Isolate(nullptr)
    {
        //
    }

    Runtime::~Runtime()
    {
        //
    }

    void Runtime::OnStart()
    {
        SetupIsolate();
        v8::Locker locker(m_Isolate);
        v8::Isolate::Scope isolateScope(m_Isolate);
        v8::HandleScope handleScope(m_Isolate);

        SetupContext();
        v8::Context::Scope scope(m_Context.Get(m_Isolate));

        SetupGlobals();
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

        fw::Logger::Get("js")->Info("tick..");
        v8::platform::PumpMessageLoop(m_Platform.get(), m_Isolate);
    }

    sdk::Result Runtime::OnHandleResourceLoad(sdk::ResourceInformation* information)
    {
        fw::Logger::Get("js")->Info("NEW RESOURCE LOADED {} {}", information->m_Path, information->m_MainFile);
        sdk::IResourceFactory* resourceFactory = sdk::IResourceFactory::GetInstance();
        Resource* newResource = new Resource(information);

        sdk::Result resourceCreationResult = resourceFactory->RegisterResource(newResource);
        m_LoadedResources.push_back(newResource);
        return {true};
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

    void Runtime::SetupContext()
    {
        v8helper::Namespace::Initialize(m_Isolate);
        v8helper::Module::Initialize(m_Isolate);
        v8helper::Class::Initialize(m_Isolate);

        auto microtaskQueue = v8::MicrotaskQueue::New(m_Isolate, v8::MicrotasksPolicy::kExplicit);
        v8::Local<v8::Context> _context = v8::Context::New(m_Isolate, nullptr, v8::Local<v8::ObjectTemplate>(), v8::Local<v8::Value>(), nullptr, microtaskQueue.get());
        m_Context.Reset(m_Isolate, _context);
        Assert(!m_Context.IsEmpty(), "Failed to create context");
    }

    void Runtime::SetupGlobals()
    {
        v8helper::Object global = m_Context.Get(m_Isolate)->Global();
        global.SetMethod("print", Print);
    }
} // namespace js
