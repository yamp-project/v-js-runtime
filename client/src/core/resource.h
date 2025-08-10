#ifndef RESOURCE_H
#define RESOURCE_H

#include <map>

#include "util/logger.h"

#include <yamp-sdk/sdk.h>
#include <v8.h>

namespace js {
    class Resource {
    public:
        void OnStart();
        void OnStop();
        void OnTick();
        void OnEvent(CoreEventType type, CAnyArray* args);
        void OnEvent(const char* name, CAnyArray* args);

        Resource(ILookupTable* lookupTable, IResource* resource, v8::Isolate* isolate);
        ~Resource() = default;

        std::string GetResourcePath() {
            return m_Resource->path;
        }

        v8::MaybeLocal<v8::Module> GetProcessedModule(const std::string &specifier) {
            if (m_ProcessedModules.contains(specifier)) {
                return m_ProcessedModules[specifier].Get(m_Isolate);
            }

            return {};
        }

        void AddProcessedModule(const std::string &specifier, v8::Local<v8::Module> module) {
            m_ProcessedModules[specifier].Reset(m_Isolate, module);
        }

    private:
        static v8::MaybeLocal<v8::Module> ResolveCallback(v8::Local<v8::Context> context,
                                                v8::Local<v8::String> specifier,
                                                v8::Local<v8::FixedArray> import_attributes,
                                                v8::Local<v8::Module> referrer);

        std::map<std::string, v8::Global<v8::Module>> m_ProcessedModules;

        IResource* m_Resource;
        Logger m_Logger;

        v8::Isolate* m_Isolate;
    };
} // js

#endif //RESOURCE_H