#include "Resource.h"

#include "helpers/misc.h"
#include "io/subprocess.h"
#include "io/files.h"

#include "natives/NativesWrapper.h"
#include "events/EventManager.h"
#include "Scheduler.h"
#include "ExceptionHandler.h"
#include "bindings/globals.h"
#include <v8helper.h>

static v8::MaybeLocal<v8::Module> DefaultImportCallback(v8::Local<v8::Context>, v8::Local<v8::String>, v8::Local<v8::FixedArray>, v8::Local<v8::Module>)
{
    return v8::MaybeLocal<v8::Module>();
}

namespace js
{
    // clang-format off
    Resource::Resource(v8::Isolate* isolate, sdk::ResourceInformation* infos, bool isTypescript) :
        m_Isolate(isolate),
        m_ResourceInformations(infos),
        m_IsTypescript(isTypescript),
        m_State(false),
        m_ExceptionHandler(std::make_unique<ExceptionHandler>(this)),
        m_Events(std::make_unique<EventManager>(this)),
        m_Scheduler(std::make_unique<Scheduler>(this))
    // clang-format on
    {
        std::filesystem::path resourcePath = infos->m_Path;
        m_mainFilePath = (resourcePath / infos->m_MainFile).string();
        Log().Info("path -> {}", m_mainFilePath);

        v8::Locker locker(isolate);
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);
        SetupContext();

        v8::Local<v8::Context> context = m_Context.Get(isolate);
        v8::Context::Scope scope(context);

        RegisterNatives();
        SetupGlobals();
    }

    Resource::~Resource()
    {
        m_Context.Reset();
    }

    void Resource::SetupContext()
    {
        v8helper::Namespace::Initialize(m_Isolate);
        v8helper::Module::Initialize(m_Isolate);
        v8helper::Class::Initialize(m_Isolate);

        m_MicrotaskQueue = v8::MicrotaskQueue::New(m_Isolate, v8::MicrotasksPolicy::kExplicit);
        v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr, v8::Local<v8::ObjectTemplate>(), v8::Local<v8::Value>(), nullptr, m_MicrotaskQueue.get());
        m_Context.Reset(m_Isolate, context);

        helpers::Assert(!m_Context.IsEmpty(), "Failed to create context");
    }

    void Resource::SetupGlobals()
    {
        v8helper::Object global = m_Context.Get(m_Isolate)->Global();
        global.SetMethod("onCore", EventManager::OnCore);

        global.SetMethod("setTimeout", bindings::global::SetTimeout);
        global.SetMethod("setInterval", bindings::global::SetInterval);
        global.SetMethod("print", bindings::global::Print);
    }

    void Resource::RegisterNatives()
    {
        sdk::INativeReflectionFactory* nativeReflectionFactory = sdk::INativeReflectionFactory::GetInstance();
        v8helper::Object global = m_Context.Get(m_Isolate)->Global();

        for (auto& native : nativeReflectionFactory->GetListOfNatives())
        {
            sdk::NativeInformation* nativeInformation = nativeReflectionFactory->GetNativeInformation(native);
            if (nativeInformation)
            {
                v8::Local<v8::FunctionTemplate> callback = v8::FunctionTemplate::New(m_Isolate, NativesWrapper::InvokeNative, v8::External::New(m_Isolate, nativeInformation));
                global.SetMethod(helpers::ToCamelCase(nativeInformation->m_Name), callback);
            }
        }
    }

    bool Resource::RunCode(std::string_view filePath)
    {
        std::optional<std::string> result = io::ReadFile(filePath, m_IsTypescript);
        if (!result)
        {
            Log().Error("Failed to read file: {}", filePath);
            return false;
        }

        v8::TryCatch tryCatch(m_Isolate);

        v8::ScriptOrigin origin{m_Isolate, v8helper::String(m_ResourceInformations->m_Name), 0, 0, false, 0, v8::Local<v8::Value>(), false, false, true, v8::Local<v8::PrimitiveArray>()};
        v8::ScriptCompiler::Source compilerSource{v8helper::String(*result), origin};
        v8::MaybeLocal<v8::Module> maybeModule = v8::ScriptCompiler::CompileModule(m_Isolate, &compilerSource);
        if (maybeModule.IsEmpty())
        {
            Log().Error("Failed to compile file");
            if (tryCatch.HasCaught())
            {
                tryCatch.Reset();
            }

            return false;
        }

        v8::Local<v8::Module> mod = maybeModule.ToLocalChecked();
        v8::Maybe<bool> instantiated = mod->InstantiateModule(m_Context.Get(m_Isolate), DefaultImportCallback);
        if (!instantiated.FromMaybe(false) || tryCatch.HasCaught())
        {
            Log().Error("Failed to instantiate module");

            if (mod->GetStatus() == v8::Module::kErrored)
            {
                v8helper::Object exception = mod->GetException().As<v8::Object>();
                Log().Error("{}", exception.Get<std::string>("message"));

                if (std::string stack = exception.Get<std::string>("stack"); !stack.empty())
                {
                    Log().Error("{}", stack);
                }
            }

            if (tryCatch.HasCaught())
            {
                tryCatch.Reset();
            }

            return false;
        }

        v8::MaybeLocal<v8::Value> maybeResult = mod->Evaluate(m_Context.Get(m_Isolate));
        if (maybeResult.IsEmpty() || maybeResult.ToLocalChecked().As<v8::Promise>()->State() == v8::Promise::PromiseState::kRejected)
        {
            Log().Error("Failed to evaluate module");

            if (mod->GetStatus() == v8::Module::kErrored)
            {
                v8helper::Object exception = mod->GetException().As<v8::Object>();
                Log().Error("{}", exception.Get<std::string>("message"));

                if (std::string stack = exception.Get<std::string>("stack"); !stack.empty())
                {
                    Log().Error("{}", stack);
                }
            }

            if (tryCatch.HasCaught())
            {
                tryCatch.Reset();
            }

            return false;
        }

        return true;
    }

    sdk::Result Resource::OnStart()
    {
        V8_SCOPE(m_Isolate);
        v8::Context::Scope ctxScope(m_Context.Get(m_Isolate));

        m_State = RunCode(m_mainFilePath);
        return {m_State};
    }

    sdk::Result Resource::OnStop()
    {
        Log().Info("Resource::Stop");
        return {true};
    }

    sdk::Result Resource::OnTick()
    {
        if (m_State)
        {
            V8_SCOPE(m_Isolate);
            m_MicrotaskQueue->PerformCheckpoint(m_Isolate);
            m_ExceptionHandler->ProcessExceptions();
            m_Scheduler->ProcessTimers();
        }

        return {true};
    }
} // namespace js
