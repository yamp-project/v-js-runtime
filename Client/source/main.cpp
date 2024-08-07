#include "Runtime.h"

#include <v-sdk/factories/RuntimeFactory.hpp>
#include <fw/Logger.h>
#include <Windows.h>

namespace sdk = yamp::sdk;

BOOL WINAPI DllMain(HINSTANCE instanceDll, DWORD reason, LPVOID reserved)
{
    fw::Logger* logger = fw::Logger::Get("js");
    logger->SetPattern(fmt::format("[JS] {}", fw::Logger::MessagePattern));

    if (reason == DLL_PROCESS_ATTACH)
    {
        auto runtimeFactory = sdk::IRuntimeFactory::GetInstance();

        logger->Info("Runtime has been loaded! {}", fmt::ptr(reserved));
        logger->Info("Author: yamp");
        logger->Info("Version: 0.0.1-alpha");
        logger->Info("Runtime factory instance: {}\n", fmt::ptr(runtimeFactory));

        js::Runtime* runtime = js::Runtime::GetInstance();
        sdk::Result result = runtimeFactory->RegisterRuntime(runtime);

        result = runtimeFactory->RegisterRuntimeResourceHandling("js", runtime);
        runtime->OnStart();
    }

    if (reason == DLL_PROCESS_DETACH)
    {
        logger->Info("Runtime is unloading...");
    }

    return true;
}
