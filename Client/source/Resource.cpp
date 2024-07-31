#include "Resource.h"
#include <v-sdk/factories/NativeFactory.hpp>
#include <v-sdk/misc/Vector.hpp>

namespace js
{
    Resource::Resource(sdk::ResourceInformation* resourceInformation) : m_ResourceInformation(resourceInformation)
    {
        //
    }

    Resource::~Resource()
    {
        //
    }

    sdk::Result Resource::OnStart()
    {
        return {true};
    }

    sdk::Result Resource::OnStop()
    {
        printf("Resource::OnStop\n");
        return {true};
    }

    sdk::Result Resource::OnTick()
    {
        return {true};
    }
} // namespace js
