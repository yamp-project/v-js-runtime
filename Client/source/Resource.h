#pragma once

#include <v-sdk/Resource.hpp>

namespace js
{
    namespace sdk = yamp::sdk;

    class Resource : public sdk::IResourceBase
    {
    public:
        Resource(sdk::ResourceInformation* information);
        virtual ~Resource();

        inline sdk::ResourceInformation* GetResourceInformation() override
        {
            return m_ResourceInformation;
        };

        sdk::Result OnStart() override;
        sdk::Result OnStop() override;
        sdk::Result OnTick() override;

    private:
        sdk::ResourceInformation* m_ResourceInformation;
    };
} // namespace js
