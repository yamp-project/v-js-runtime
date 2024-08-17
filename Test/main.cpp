#include "Runtime.h"
#include "Resource.h"

#include <v-sdk/Resource.hpp>

int main()
{
    js::Runtime* rt = js::Runtime::GetInstance();
    rt->OnStart();

    yamp::sdk::ResourceInformation infos{};
    infos.m_Name = "Test";
    infos.m_Path = "D:/.yamp/v-js-runtime/bin/win64/Release";
    infos.m_MainFile = "main.js";
    printf("Hello\n");

    js::Resource* resource = new js::Resource(rt->GetIsolate(), &infos, false);
    resource->OnStart();

    while (true)
    {
        rt->OnTick();
    }

    return 0;
}
