#include "resource.h"

#include <filesystem>

namespace js
{
    Resource::Resource(ILookupTable* lookupTable, IResource* resource, v8::Isolate* isolate) : m_Resource(resource), m_Logger(Logger(lookupTable, std::format("resource {}", resource->name))), m_Isolate(isolate)
    {
    }

    void Resource::OnStart()
    {
        std::filesystem::path resourcePath = m_Resource->resourcePath;
        std::filesystem::path mainFilePath = m_Resource->resourceMainFile;

        v8::Isolate::Scope scope(m_Isolate);

        m_Logger.Info("Running resource %s with main file %s and resource path %s!", m_Resource->name, mainFilePath, resourcePath);
    }

    void Resource::OnStop()
    {
        //
    }

    void Resource::OnTick()
    {
        //
    }
} // js