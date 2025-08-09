#include "resource.h"

#include <filesystem>

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
        v8::Local<v8::Context> context = v8::Context::New(m_Isolate);

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

        v8::Maybe<bool> result = scriptModule->InstantiateModule(context, Resource::ResolveCallback, nullptr);

        //

        m_Logger.Info("Running resource %s with main file %s and resource path %s!", m_Resource->name, mainFilePath.c_str(), resourcePath.c_str());
    }

    void Resource::OnStop()
    {
        //
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

    v8::MaybeLocal<v8::Module> ResolveCallback(v8::Local<v8::Context> context,
        v8::Local<v8::String> specifier, v8::Local<v8::FixedArray> import_attributes, v8::Local<v8::Module> referrer) {

        auto* selfRef = static_cast<Resource*>(context->GetAlignedPointerFromEmbedderData(1));

        std::string resourcePath = selfRef->GetResourcePath();
    }
} // js