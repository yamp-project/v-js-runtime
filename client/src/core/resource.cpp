#include "resource.h"

#include <filesystem>

#include "JavaScriptCore/JavaScript.h"

#include "runtime.h"
#include "util/utils.h"
#include "../interop/resource/resource_object.h"


namespace js {
    void SetupGlobalObjects(JSGlobalContextRef context, Resource *resource) {
        JSObjectRef globalObject = JSContextGetGlobalObject(context);

        JSObjectRef yampObject = JSObjectMake(context, nullptr, nullptr);

        JSObjectRef resourceObject = interop::ResourceObject::CreateResourceObject(context, resource);

        // add resource object to yamp object
        JSStringRef resourceName = JSStringCreateWithUTF8CString("resource");
        JSObjectSetProperty(context, yampObject, resourceName, resourceObject, kJSPropertyAttributeReadOnly, nullptr);
        JSStringRelease(resourceName);

        // Add yamp object to global
        JSStringRef yampName = JSStringCreateWithUTF8CString("yamp");
        JSObjectSetProperty(context, globalObject, yampName, yampObject, kJSPropertyAttributeReadOnly, nullptr);
        JSStringRelease(yampName);
    }

    Resource::Resource(SDK_Interface *lookupTable, SDK_Resource *resource) : m_Resource(resource), m_Logger(Logger(lookupTable, std::format("resource {}", resource->name))), m_ContextWrapper(std::make_unique<wrapper::JSContextWrapper>()){
        SetupGlobalObjects(m_ContextWrapper.get()->get(), this);
    }

    void Resource::OnStart()
    {
        const std::filesystem::path resourcePath = m_Resource->path;
        const std::filesystem::path mainFilePath = m_Resource->mainFile;

        m_Logger.Info("Running resource %s with main file %s and resource path %s!", m_Resource->name, mainFilePath.c_str(), resourcePath.c_str());

        m_ModuleLoader = std::make_unique<wrapper::JSModuleLoader>(m_ContextWrapper->get(), resourcePath, &m_Logger);

        if (!m_ModuleLoader->LoadMainFile(mainFilePath)) {
            m_Logger.Error("Failed to load main file!");
        }
    }

    void Resource::OnStop()
    {
        m_Logger.Info("Stopping resource %s", m_Resource->name);
        m_ModuleLoader.reset();
        this->~Resource();
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
            JSObjectCallAsFunction(
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