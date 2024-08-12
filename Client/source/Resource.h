#pragma once

#include <v-sdk/Resource.hpp>
#include <fw/Logger.h>
#include <v8helper.h>
#include <v8.h>

namespace js
{
    namespace sdk = yamp::sdk;

    class EventManager;
    class Resource : public sdk::IResourceBase
    {
    public:
        Resource(sdk::ResourceInformation* information, v8::Isolate* isolate);
        virtual ~Resource();

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
        v8::Isolate* m_Isolate;
        EventManager* m_Events;

        v8helper::Persistent<v8::Context> m_Context;
        std::string m_mainFilePath;

        void SetupContext();
        void SetupGlobals();
        bool RunCode(std::string_view jsFilePath);
    };
} // namespace js
