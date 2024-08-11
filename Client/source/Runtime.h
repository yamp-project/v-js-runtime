#pragma once

#include <v-sdk/Runtime.hpp>
#include <fw/utils/InstanceBase.h>
#include <fw/Logger.h>

#include <v8.h>

namespace js
{
    namespace sdk = yamp::sdk;

    class Resource;
    class Runtime : public fw::utils::InstanceBase, public sdk::IRuntimeBase
    {
    public:
        Runtime();
        ~Runtime();

        inline const char* GetName() override
        {
            return "js";
        }

        inline const char* GetDescription() override
        {
            return "Very second js runtime in yamp.";
        }

        void OnStart() override;
        void OnStop() override;
        void OnTick() override;

        sdk::Result OnHandleResourceLoad(sdk::ResourceInformation* Information) override;

        [[nodiscard]] inline fw::Logger& Log()
        {
            return *fw::Logger::Get("JS::Runtime");
        }

        IMPLEMENT_INSTANCE_FUNCTION(Runtime);

    private:
        std::vector<Resource*> m_LoadedResources;
        std::unique_ptr<v8::Platform> m_Platform;
        v8::Isolate* m_Isolate;

        void SetupIsolate();
    };
} // namespace js
