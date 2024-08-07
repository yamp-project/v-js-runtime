#pragma once

#include <fw/utils/InstanceBase.h>
#include <v-sdk/Runtime.hpp>

#include <v8helper.h>
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

        [[nodiscard]] inline v8::Isolate* GetIsolate() const
        {
            return m_Isolate;
        }

        [[nodiscard]] inline v8helper::Persistent<v8::Context> GetContext() const
        {
            return m_Context;
        }

        IMPLEMENT_INSTANCE_FUNCTION(Runtime);

    private:
        v8helper::Persistent<v8::Context> m_Context;
        std::unique_ptr<v8::Platform> m_Platform;
        v8::Isolate* m_Isolate;

        std::vector<Resource*> m_LoadedResources;

        void SetupIsolate();
        void SetupContext();
        void SetupGlobals();
    };
} // namespace js
