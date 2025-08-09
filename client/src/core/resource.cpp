#include "resource.h"

#include <filesystem>

namespace js
{
    Resource::Resource(ILookupTable* lookupTable, IResource* resource, v8::Isolate* isolate) : m_Resource(resource), m_Logger(Logger(lookupTable, std::format("resource {}", resource->name))), m_Isolate(isolate)
    {
    }

    void Resource::OnStart()
    {
        std::filesystem::path resourcePath = m_Resource->path;
        std::filesystem::path mainFilePath = m_Resource->mainFile;

        v8::Isolate::Scope scope(m_Isolate);

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
} // js