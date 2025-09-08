#ifndef RESOURCE_H
#define RESOURCE_H

#include <map>

#include "util/logger.h"

#include <yamp-sdk/sdk.h>
#include <JavaScriptCore/JavaScript.h>

#include "runtime.h"
#include "../wrapper/js_context_wrapper.h"
#include "../wrapper/js_module_loader.h"

namespace js {
    class Resource {
    public:
        void OnStart();
        void OnStop();
        void OnTick();
        void OnEvent(CoreEventType type, const CAnyArray* args);
        void OnEvent(const char* name, const CAnyArray* args);

        Resource(ILookupTable* lookupTable, IResource* resource)
                    : m_Resource(resource), m_Logger(Logger(lookupTable, std::format("resource {}", resource->name))), m_ContextWrapper(std::make_unique<wrapper::JSContextWrapper>(GetGlobalTemplate())) {}

        ~Resource() = default;

        [[nodiscard]]
        std::string GetResourcePath() const {
            return m_Resource->path;
        }

        [[nodiscard]]
        std::string GetMainFilePath() const {
            return m_Resource->mainFile;
        }

        [[nodiscard]]
        std::string GetResourceName() const {
            return m_Resource->name;
        }

        void AddCoreEventCallback(const CoreEventType event, const JSObjectRef callback) {
            m_CoreEventCallbacks[event].push_back(callback);
        }

        void AddEventCallback(const std::string& event, const JSObjectRef callback) {
            m_EventCallbacks[event].push_back(callback);
        }

    private:
        void handleEvent(const std::vector<JSObjectRef>& functions, const CAnyArray* args);

    private:
        IResource* m_Resource;
        Logger m_Logger;

        std::map<CoreEventType, std::vector<JSObjectRef>> m_CoreEventCallbacks;
        std::map<std::string, std::vector<JSObjectRef>> m_EventCallbacks;

        std::unique_ptr<wrapper::JSContextWrapper> m_ContextWrapper;
        std::unique_ptr<wrapper::JSModuleLoader> m_ModuleLoader;
    };
} // js

#endif //RESOURCE_H