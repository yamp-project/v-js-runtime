#include "resource.h"

#include <filesystem>

#include "../template/global_template.h"
#include "util/utils.h"

namespace js {
    Resource::Resource(ILookupTable* lookupTable, IResource* resource, v8::Isolate* isolate) : m_Resource(resource), m_Logger(Logger(lookupTable, std::format("resource {}", resource->name))), m_Isolate(isolate)
    {
    }

    void Resource::OnStart()
    {
        std::filesystem::path resourcePath = m_Resource->path;
        std::filesystem::path mainFilePath = m_Resource->mainFile;

        v8::Isolate::Scope scope(m_Isolate);
        v8::HandleScope handleScope(m_Isolate);
        v8::Local<v8::Context> context = v8::Context::New(m_Isolate, nullptr);

        context->SetAlignedPointerInEmbedderData(1, this);

        v8::Context::Scope contextScope(context);

        v8::Local<v8::String> jsSource = utils::StringToV8(m_Isolate, utils::ReadFile(m_Resource->mainFile));

        v8::ScriptCompiler::Source scriptSource(jsSource);
        v8::Local<v8::Module> scriptModule;
        if (!v8::ScriptCompiler::CompileModule(m_Isolate, &scriptSource).ToLocal(&scriptModule)) {
            m_Logger.Warn("Failed to compile script");
            this->OnStop();
            return;
        }

        const v8::Maybe<bool> result = scriptModule->InstantiateModule(context, Resource::ResolveCallback, nullptr);
        if (result.IsNothing()) {
            m_Logger.Warn("Failed to instantiate module");
            this->OnStop();
            return;
        }

        // This runs the javascript code
        v8::Local<v8::Value> moduleResult;
        if (!scriptModule->Evaluate(context).ToLocal(&moduleResult)) {
            m_Logger.Warn("Failed to evaluate script");
            this->OnStop();
            return;
        }

        m_Logger.Info("Running resource %s with main file %s and resource path %s!", m_Resource->name, mainFilePath.c_str(), resourcePath.c_str());
    }

    void Resource::OnStop()
    {
        if (m_Isolate) {
            m_Isolate->TerminateExecution();
        }
    }

    void Resource::OnTick()
    {
        //
    }

    void Resource::OnEvent(CoreEventType type, CAnyArray* args)
    {
    }

    void Resource::OnEvent(const char* name, CAnyArray* args)
    {
    }

    v8::MaybeLocal<v8::Module> Resource::ResolveCallback(const v8::Local<v8::Context> context,
        const v8::Local<v8::String> specifier, const v8::Local<v8::FixedArray> import_attributes, const v8::Local<v8::Module> referrer) {

        v8::Isolate* isolate = context->GetIsolate();

        auto* selfRef = static_cast<Resource*>(context->GetAlignedPointerFromEmbedderData(1));

        const std::string resourcePath = selfRef->GetResourcePath();

        std::string specifierStr (*v8::String::Utf8Value(isolate, specifier));

        v8::MaybeLocal<v8::Module> processedModule = selfRef->GetProcessedModule(specifierStr);

        if (!processedModule.IsEmpty()) {
            return processedModule;
        }

        if (specifierStr.length() > 1 && specifierStr[0] == '.' && specifierStr[1] == '/') {
            specifierStr = specifierStr.substr(1);
        }

        const std::string src = utils::ReadFile(resourcePath  + specifierStr);
        if (src.empty()) {
            return {};
        }

        v8::ScriptCompiler::Source scriptSource(utils::StringToV8(isolate, src));
        v8::Local<v8::Module> scriptModule;
        if (!v8::ScriptCompiler::CompileModule(isolate, &scriptSource).ToLocal(&scriptModule)) {
            return {};
        }

        selfRef->AddProcessedModule(specifierStr, scriptModule);

        return {scriptModule};
    }
} // js