#pragma once

#include <v-sdk/Runtime.hpp>
#include <vector>

namespace js
{
    namespace sdk = yamp::sdk;

    class Resource;
    class Runtime : public sdk::IRuntimeBase
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

    private:
        std::vector<Resource*> m_LoadedResources;
    };
} // namespace js
