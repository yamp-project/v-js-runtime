#pragma once

#include "Timers.h"
#include "stdafx.h"

#include <v-sdk/Resource.hpp>
#include <fw/Logger.h>
#include <memory>

namespace js
{
    namespace sdk = yamp::sdk;

    class ExceptionHandler;
    class EventManager;
    class Scheduler;

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

        inline v8::Isolate* GetIsolate() const
        {
            return m_Isolate;
        }

        inline EventManager& GetEventManager() const
        {
            return *m_Events;
        }

        inline Scheduler& GetScheduler() const
        {
            return *m_Scheduler;
        }

        inline v8::Local<v8::Context> GetContext() const
        {
            return m_Context.Get(m_Isolate);
        }

        inline fw::Logger& Log()
        {
            return *fw::Logger::Get(fmt::format("JS::{}", m_ResourceInformations->m_Name));
        }

        inline ExceptionHandler& GetExceptionHandler() const
        {
            return *m_ExceptionHandler;
        }

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
        std::unique_ptr<ExceptionHandler> m_ExceptionHandler;
        std::unique_ptr<EventManager> m_Events;
        std::unique_ptr<Scheduler> m_Scheduler;

        v8::Persistent<v8::Context> m_Context;
        std::string m_mainFilePath;

        void SetupContext();
        void SetupGlobals();

        bool RunCode(std::string_view jsFilePath);
        void RegisterNatives();
    };
} // namespace js
