#include "Runtime.h"

#include "core/ExceptionHandler.h"
#include "core/EventManager.h"
#include "helpers/misc.h"
#include "Resource.h"

// TODO: remove
#include <Windows.h>
#include <v-sdk/factories/ResourceFactory.hpp>
#include <fw/Logger.h>

#define V8_SCOPE(isolate)                     \
    v8::Locker locker(isolate);               \
    v8::Isolate::Scope isolateScope(isolate); \
    v8::HandleScope scope(isolate)

static void MessageListener(v8::Local<v8::Message> message, v8::Local<v8::Value> error)
{
    fw::Logger::Get("DEBUG")->Warn("[JS] V8 message received! {}", "");
}

namespace js
{
    void Runtime::OnEvent(yamp::sdk::AnyBuiltinEvent* ev)
    {
        Runtime* runtime = Runtime::GetInstance();
        V8_SCOPE(runtime->GetIsolate());

        std::vector<v8::Local<v8::Value>> v8Args;
        v8Args.reserve(ev->size);

        for (size_t i = 0; i < ev->size; ++i)
        {
            yamp::sdk::AnyValue arg = ev->args[i];

            switch (arg.m_Type)
            {
            case yamp::sdk::AnyValue::Type::DOUBLE:
                v8Args.push_back(v8helper::JSValue(arg.As<double>()));
                break;

            case yamp::sdk::AnyValue::Type::BOOL:
                v8Args.push_back(v8helper::JSValue(arg.As<bool>()));
                break;

            case yamp::sdk::AnyValue::Type::STRING:
                v8Args.push_back(v8helper::String(arg.As<char*>()));
                break;

            default:
                runtime->Log().Warn("unsupported AnyValue::Type {}", (uint8_t)arg.m_Type);
                break;
            }
        }

        for (Resource* resource : runtime->GetResources())
        {
            resource->GetEventManager().DispatchEvent(EventType::CORE, ev->eventName, v8Args);
        }
    }

    Runtime::Runtime() : m_Platform(v8::platform::NewDefaultPlatform()), m_Resources(), m_Isolate(nullptr)
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
        v8::V8::InitializePlatform(m_Platform.get());
        v8::V8::InitializeICU("D:/.yamp/v-client/bin/runtimes/icudtl_v8.dat");
        v8::V8::Initialize();

        v8::Isolate::CreateParams createParams;
        createParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        createParams.allow_atomics_wait = false;

        m_Isolate = v8::Isolate::New(createParams);
        helpers::Assert(m_Isolate != nullptr, "Failed to create isolate");

        m_Isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
        m_Isolate->SetCaptureStackTraceForUncaughtExceptions(true, 5);

        m_Isolate->SetFatalErrorHandler(OnFatalError);
        m_Isolate->SetOOMErrorHandler(OnHeapOOM);
        m_Isolate->AddNearHeapLimitCallback(OnNearHeapLimit, nullptr);
        m_Isolate->SetPromiseRejectCallback(ExceptionHandler::OnPromiseRejected);
        // m_Isolate->SetHostImportModuleDynamicallyCallback(ImportModuleDynamically);
        // m_Isolate->SetHostInitializeImportMetaObjectCallback(InitializeImportMetaObject);
        m_Isolate->AddMessageListener(MessageListener);
    }

    void Runtime::OnStart()
    {
        SetupIsolate();

        Log().Info("Succesfully started");

        // TODO: move to the sdk
        HMODULE clientModule = GetModuleHandleA("yamp_client.dll");
        auto RegisterOnEventCallback = reinterpret_cast<void (*)(void(yamp::sdk::AnyBuiltinEvent*))>(GetProcAddress(clientModule, "OnEvent"));
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

    Resource* Runtime::GetResourceByContext(const v8::Local<v8::Context>& context) const
    {
        for (auto resource : m_Resources)
        {
            if (resource->GetContext() == context)
            {
                return resource;
            }
        }

        return nullptr;
    }

    size_t Runtime::OnNearHeapLimit(void*, size_t current, size_t initial)
    {
        Runtime::GetInstance()->Log().Warn("The remaining V8 heap space is approaching critical levels. Increasing heap limit...");

        // Increase the heap limit by 100MB if the heap limit has not exceeded 4GB
        uint64_t currentLimitMb = (current / 1024) / 1024;
        return currentLimitMb < 4096 ? current + (100 * 1024 * 1024) : current;
    }

    void Runtime::OnHeapOOM(const char* location, bool isHeap)
    {
        if (isHeap)
        {
            Runtime::GetInstance()->Log().Error("V8 heap out of memory! {}", location);
        }
    }

    void Runtime::OnFatalError(const char* location, const char* message)
    {
        Runtime::GetInstance()->Log().Error("V8 fatal error! {} {}", location, message);
    }
} // namespace js
