#include "resource.h"

#include <filesystem>

#include "JavaScriptCore/JavaScript.h"

#include "runtime.h"
#include "util/utils.h"

namespace js {
    void Resource::OnStart()
    {
        const std::filesystem::path resourcePath = m_Resource->path;
        const std::filesystem::path mainFilePath = m_Resource->mainFile;

        m_Logger.Info("Running resource %s with main file %s and resource path %s!", m_Resource->name, mainFilePath.c_str(), resourcePath.c_str());
    }

    void Resource::OnStop()
    {

    }

    void Resource::OnTick()
    {
        //
    }

    void Resource::OnEvent(const CoreEventType type, const CAnyArray* args)
    {
        handleEvent(m_CoreEventCallbacks[type], args);
    }

    void Resource::OnEvent(const char* name, const CAnyArray* args)
    {
        handleEvent(m_EventCallbacks[name], args);
    }

    void Resource::handleEvent(const std::vector<JSObjectRef>& functions, const CAnyArray* args) {
        if (functions.empty()) {
            return;
        }

        std::vector<JSValueRef> funcArgs(args->size);

        for (int i = 0; args->size <= i; i++) {
            funcArgs[i] = utils::ToJSValue(m_ContextWrapper->get(), *args->buffer[i]);
        }

        for (auto function : functions) {
            if (!function) {
                continue;
            }

            JSValueRef exception = nullptr;
            JSValueRef result = JSObjectCallAsFunction(
                m_ContextWrapper->get(),
                function,
                nullptr,
                args->size,
                funcArgs.data(),
                &exception
            );

            if (exception) {
                m_Logger.Error("An error (%s) occurred while running %s", exception, function);
            }
        }
    }
} // js