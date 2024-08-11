#pragma once

#include <v-sdk/Resource.hpp>
#include <fw/Logger.h>

#include <v8helper.h>
#include <v8.h>

namespace js
{
    namespace sdk = yamp::sdk;

    class Resource : public sdk::IResourceBase
    {
    public:
        Resource(sdk::ResourceInformation* information, v8::Isolate* isolate);
        virtual ~Resource();

        [[nodiscard]] inline sdk::ResourceInformation* GetResourceInformation() override
        {
            return m_ResourceInformation;
        };

        [[nodiscard]] inline v8helper::Persistent<v8::Context> GetContext() const
        {
            return m_Context;
        }

        [[nodiscard]] inline fw::Logger& Log()
        {
            return *fw::Logger::Get(fmt::format("JS::{}", m_ResourceInformation->m_Name));
        }

        sdk::Result OnStart() override;
        sdk::Result OnStop() override;
        sdk::Result OnTick() override;

    private:
        sdk::ResourceInformation* m_ResourceInformation;
        v8::Isolate* m_Isolate;

        v8helper::Persistent<v8::Context> m_Context;
        std::string m_mainFilePath;

        void SetupContext();
        void SetupGlobals();
        bool RunCode(std::string_view jsFilePath);
    };
} // namespace js
