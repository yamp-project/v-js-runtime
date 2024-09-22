#pragma once

#include "stdafx.h"

#include <v-sdk/Resource.hpp>
#include <fw/Logger.h>
#include <v8.h>

#define IMPLEMENT_GET(ReturnType, Name, Field) \
    STRONG_INLINE ReturnType Get##Name() const \
    {                                          \
        return Field;                          \
    }

namespace core
{
    class ExceptionHandler;
    class NativeInvoker;
    class EventManager;
    class Scheduler;
} // namespace core

namespace js
{
    namespace sdk = yamp::sdk;

    class Resource : public sdk::IResourceBase
    {
    public:
        Resource(v8::Isolate* isolate, sdk::ResourceInformation* infos, bool isTypescript);
        ~Resource();

        // TODO: rename with the plural form
        inline sdk::ResourceInformation* GetResourceInformation() override
        {
            return m_ResourceInformations;
        };

        inline fw::Logger& Log()
        {
            return *fw::Logger::Get(fmt::format("JS::{}", m_ResourceInformations->m_Name));
        }

        IMPLEMENT_GET(v8::Isolate*, Isolate, m_Isolate);
        IMPLEMENT_GET(v8::Local<v8::Context>, Context, m_Context.Get(m_Isolate));

        IMPLEMENT_GET(core::EventManager&, EventManager, *m_Events);
        IMPLEMENT_GET(core::NativeInvoker&, NativeInvoker, *m_NativeInvoker);
        IMPLEMENT_GET(core::Scheduler&, Scheduler, *m_Scheduler);
        IMPLEMENT_GET(core::ExceptionHandler&, ExceptionHandler, *m_ExceptionHandler);

        sdk::Result OnStart() override;
        sdk::Result OnStop() override;
        sdk::Result OnTick() override;

    private:
        sdk::ResourceInformation* m_ResourceInformations;
        bool m_IsTypescript;
        bool m_State;

        v8::Isolate* m_Isolate;

        // extensions
        std::unique_ptr<v8::MicrotaskQueue> m_MicrotaskQueue;
        std::unique_ptr<core::ExceptionHandler> m_ExceptionHandler;
        std::unique_ptr<core::NativeInvoker> m_NativeInvoker;
        std::unique_ptr<core::EventManager> m_Events;
        std::unique_ptr<core::Scheduler> m_Scheduler;

        v8::Persistent<v8::Context> m_Context;
        std::string m_mainFilePath;

        void SetupContext();
        void SetupGlobals();

        bool RunCode(std::string_view jsFilePath);
        void RegisterNatives();
    };
} // namespace js
