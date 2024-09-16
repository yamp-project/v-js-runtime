#include "Resource.h"

#include "helpers/misc.h"
#include "io/subprocess.h"
#include "io/files.h"

#include "natives/NativesWrapper.h"
#include "events/EventManager.h"
#include "ResourceScheduler.h"
#include "bindings/globals.h"

static v8::MaybeLocal<v8::Module> DefaultImportCallback(v8::Local<v8::Context>, v8::Local<v8::String>, v8::Local<v8::FixedArray>, v8::Local<v8::Module>)
{
    return v8::MaybeLocal<v8::Module>();
}

// clang-format off
static v8helper::Module clientModule("@yamp/client", [](v8helper::ModuleTemplate& module)
{
    module.StaticProperty("isClient", true);

    // module.StaticFunction("test", v8::FunctionTemplate::New(module.GetIsolate(), Temp, v8::External::New(module.GetIsolate(), nullptr)));
}, nullptr, v8helper::Module::Option::EXPORT_AS_DEFAULT);
// clang-format on

namespace js
{
    // TODO: see with others how we should handle the lifetime of the event manager (unique_ptr, manual)
    Resource::Resource(v8::Isolate* isolate, sdk::ResourceInformation* infos, bool isTypescript)
        : m_Isolate(isolate), m_ResourceInformations(infos), m_IsTypescript(isTypescript), m_Scheduler(new ResourceScheduler(this)), m_Events(new EventManager(this))
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
        delete m_Scheduler;
        delete m_Events;
    }

    void Resource::SetupContext()
    {
        v8helper::Namespace::Initialize(m_Isolate);
        v8helper::Module::Initialize(m_Isolate);
        v8helper::Class::Initialize(m_Isolate);

        auto microtaskQueue = v8::MicrotaskQueue::New(m_Isolate, v8::MicrotasksPolicy::kExplicit);
        v8::Local<v8::Context> _context = v8::Context::New(m_Isolate, nullptr, v8::Local<v8::ObjectTemplate>(), v8::Local<v8::Value>(), nullptr, microtaskQueue.get());
        _context->SetAlignedPointerInEmbedderData(V8HELPER_MODULEHANDLER_EMBEDDER_FIELD, this);
        m_Context.Reset(m_Isolate, _context);

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

    std::optional<std::string> Resource::ReadTsFile(std::string_view filePath)
    {
        // TODO: get the path from the client
        const char* commandLine[] = {
            "D:/.yamp/v-client/bin/runtimes/esbuild.exe", "D:/.yamp/v-client/bin/resources/js_test/main.ts", "--bundle", "--format=esm", "--platform=browser", "--external:@yamp/client", NULL};
        subprocess_s process;

        int8_t options = subprocess_option_combined_stdout_stderr | subprocess_option_no_window;
        int32_t result = subprocess_create(commandLine, options, &process);
        if (result != 0)
        {
            return std::nullopt;
        }

        return io::ReadFilePipe(subprocess_stdout(&process));
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
                auto callback = v8::FunctionTemplate::New(m_Isolate, NativesWrapper::InvokeNative, v8::External::New(m_Isolate, nativeInformation));
                global.SetMethod(helpers::ToCamelCase(nativeInformation->m_Name), callback);
            }
        }
    }

    bool Resource::RunCode(std::string_view filePath)
    {
        std::optional<std::string> result = m_IsTypescript ? ReadTsFile(filePath) : io::ReadFile(filePath);
        if (!result)
        {
            Log().Error("Failed to read file: {}", filePath);
            return false;
        }

        v8::ScriptOrigin origin{m_Isolate, v8helper::String(m_ResourceInformations->m_Name), 0, 0, false, 0, v8::Local<v8::Value>(), false, false, true, v8::Local<v8::PrimitiveArray>()};
        v8::ScriptCompiler::Source compilerSource{v8helper::String(*result), origin};
        v8::MaybeLocal<v8::Module> maybeModule = v8::ScriptCompiler::CompileModule(m_Isolate, &compilerSource);

        v8::Local<v8::Module> mod;
        if (!maybeModule.ToLocal(&mod))
        {
            Log().Error("Failed to compile module");
            return false;
        }

        v8::Maybe<bool> instantiated = mod->InstantiateModule(m_Context.Get(m_Isolate), DefaultImportCallback);
        if (!instantiated.FromMaybe(false) || mod->GetStatus() != v8::Module::Status::kInstantiated)
        {
            Log().Error("Failed to instantiate module");
            return false;
        }

        auto _ = mod->Evaluate(m_Context.Get(m_Isolate));
        if (mod->GetStatus() != v8::Module::Status::kEvaluated)
        {
            Log().Error("Failed to evaluate module");
            return false;
        }

        if (mod->GetStatus() == v8::Module::kErrored)
        {
            v8helper::Object exceptionObj = mod->GetException().As<v8::Object>();
            Log().Error("{}", exceptionObj.Get<std::string>("message"));

            std::string stack = exceptionObj.Get<std::string>("stack");
            if (!stack.empty())
            {
                Log().Error("{}", stack);
            }

            return false;
        }

        return true;
    }

    sdk::Result Resource::OnStart()
    {
        v8::Locker locker(m_Isolate);
        v8::Isolate::Scope isolateScope(m_Isolate);
        v8::HandleScope handleScope(m_Isolate);
        v8::Context::Scope ctxScope(m_Context.Get(m_Isolate));

        bool status = RunCode(m_mainFilePath);
        return {status};
    }

    sdk::Result Resource::OnStop()
    {
        Log().Info("Resource::Stop");
        return {true};
    }

    sdk::Result Resource::OnTick()
    {
        m_Scheduler->ProcessTimers();
        return {true};
    }
} // namespace js
