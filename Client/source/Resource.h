#pragma once

#include "Timers.h"
#include "stdafx.h"

#include <v8-microtask-queue.h>
#include <v-sdk/Resource.hpp>
#include <fw/Logger.h>
#include <v8helper.h>
#include <memory>

namespace js
{
    namespace sdk = yamp::sdk;

    class ResourceScheduler;
    class EventManager;
    class Resource : public sdk::IResourceBase, public v8helper::IModuleHandler, public v8helper::IExceptionHandler
    {
    public:
        Resource(v8::Isolate* isolate, sdk::ResourceInformation* infos, bool isTypescript);
        virtual ~Resource();

        std::string ResolvePath(v8::Local<v8::Context> context, const std::string& path, const std::string& referrer) override
        {
            Log().Info("path {} referrer {}", path, referrer);
            return "";
        }

        std::vector<uint8_t> ReadFile(v8::Local<v8::Context> context, const std::string& path) override
        {
            return std::vector<uint8_t>();
        }

        void HandleEvent(ExceptionEvent event, std::string_view rejectionMessage) override
        {
            Log().Warn("HandleEvent {}", rejectionMessage);
        }

        void HandleRejection(PromiseRejection& rejection) override
        {
            Log().Warn("HandleRejection {}", rejection.GetRejectionMessage());
        }

        // TODO: rename with the plural form
        [[nodiscard]] inline sdk::ResourceInformation* GetResourceInformation() override
        {
            return m_ResourceInformations;
        };

        [[nodiscard]] inline v8::Isolate* GetIsolate() const
        {
            return m_Isolate;
        }

        [[nodiscard]] inline EventManager* GetEventManager() const
        {
            return m_Events;
        }

        [[nodiscard]] inline ResourceScheduler* GetScheduler() const
        {
            return m_Scheduler;
        }

        [[nodiscard]] inline v8helper::Persistent<v8::Context> GetContext() const
        {
            return m_Context;
        }

        [[nodiscard]] inline fw::Logger& Log()
        {
            return *fw::Logger::Get(fmt::format("JS::{}", m_ResourceInformations->m_Name));
        }

        sdk::Result OnStart() override;
        sdk::Result OnStop() override;
        sdk::Result OnTick() override;

    private:
        sdk::ResourceInformation* m_ResourceInformations;
        bool m_IsTypescript;
        bool m_State;

        v8::Isolate* m_Isolate;
        ResourceScheduler* m_Scheduler;
        EventManager* m_Events;

        std::unique_ptr<v8::MicrotaskQueue> m_MicrotaskQueue;
        v8helper::Persistent<v8::Context> m_Context;
        std::string m_mainFilePath;

        void SetupContext();
        void SetupGlobals();

        std::optional<std::string> ReadTsFile(std::string_view filePath);
        bool RunCode(std::string_view jsFilePath);
        void RegisterNatives();
    };
} // namespace js
