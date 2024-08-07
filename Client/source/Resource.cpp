#include "Resource.h"
#include "Runtime.h"
#include "helpers.h"

#include <filesystem>
#include <iostream>
#include <fstream>

#include <fw/Logger.h>
#include <v8helper.h>
#include <v8.h>

static v8::MaybeLocal<v8::Module> DefaultImportCallback(v8::Local<v8::Context>, v8::Local<v8::String>, v8::Local<v8::FixedArray>, v8::Local<v8::Module>)
{
    return v8::MaybeLocal<v8::Module>();
}

namespace js
{
    Resource::Resource(sdk::ResourceInformation* resourceInformation) : m_ResourceInformation(resourceInformation)
    {
        //
    }

    Resource::~Resource()
    {
        //
    }

    sdk::Result Resource::OnStart()
    {
        std::filesystem::path resourcePath = m_ResourceInformation->m_Path;
        std::filesystem::path mainFilePath = resourcePath / m_ResourceInformation->m_MainFile;

        std::string mainFilePathStr = mainFilePath.string();
        if (!mainFilePathStr.ends_with(".js"))
        {
            std::cerr << "No valid js file extension: " << mainFilePathStr << std::endl;
            return {false};
        }

        auto result = ReadFile(mainFilePathStr);
        if (!result.has_value())
        {
            std::cerr << "Failed to read file: " << mainFilePathStr << std::endl;
            return {false};
        }

        Runtime* runtime = Runtime::GetInstance();
        auto context = runtime->GetContext();
        auto isolate = runtime->GetIsolate();

        v8::Locker locker(isolate);
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);
        v8::Context::Scope ctxScope(context.Get(isolate));

        v8::ScriptOrigin origin{isolate, v8helper::String("<script>"), 0, 0, false, 0, v8::Local<v8::Value>(), false, false, true, v8::Local<v8::PrimitiveArray>()};
        v8::ScriptCompiler::Source compilerSource{v8helper::String(result.value()), origin};
        v8::MaybeLocal<v8::Module> maybeModule = v8::ScriptCompiler::CompileModule(isolate, &compilerSource);
        v8::Local<v8::Module> mod;
        Assert(maybeModule.ToLocal(&mod), "Failed to compile module");

        v8::Maybe<bool> instantiated = mod->InstantiateModule(context.Get(isolate), DefaultImportCallback);
        Assert(instantiated.FromMaybe(false) && mod->GetStatus() == v8::Module::Status::kInstantiated, "Failed to instantiate module");

        auto _ = mod->Evaluate(context.Get(isolate));
        Assert(mod->GetStatus() == v8::Module::Status::kEvaluated, "Failed to evaluate module", false);

        if (mod->GetStatus() == v8::Module::kErrored)
        {
            v8helper::Object exceptionObj = mod->GetException().As<v8::Object>();
            std::cout << "[JS] " << exceptionObj.Get<std::string>("message") << std::endl;
            std::string stack = exceptionObj.Get<std::string>("stack");
            if (!stack.empty())
            {
                std::cout << stack << std::endl;
            }

            return {false};
        }

        return {true};
    }

    sdk::Result Resource::OnStop()
    {
        printf("Resource::OnStop\n");
        return {true};
    }

    sdk::Result Resource::OnTick()
    {
        return {true};
    }
} // namespace js
