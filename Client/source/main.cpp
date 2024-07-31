#include <v-sdk/factories/RuntimeFactory.hpp>
#include <fw/Logger.h>
#include "Runtime.h"

#include <Windows.h>
#include <iostream>

namespace sdk = yamp::sdk;

BOOL WINAPI DllMain(HINSTANCE instanceDll, DWORD reason, LPVOID reserved)
{
    fw::Logger* logger = fw::Logger::Get("js");
    logger->SetPattern(fmt::format("[JS] %s", fw::Logger::MessagePattern));

    if (reason == DLL_PROCESS_ATTACH)
    {
        auto runtimeFactory = sdk::IRuntimeFactory::GetInstance();

        logger->Info("Runtime has been loaded! {}", fmt::ptr(reserved));
        logger->Info("Author: yamp");
        logger->Info("Version: 0.0.1-alpha");
        logger->Info("Runtime factory instance: {}\n", fmt::ptr(runtimeFactory));

        js::Runtime* runtime = new js::Runtime;
        sdk::Result result = runtimeFactory->RegisterRuntime(runtime);
        result = runtimeFactory->RegisterRuntimeResourceHandling("js", runtime);
    }

    if (reason == DLL_PROCESS_DETACH)
    {
        logger->Info("Runtime is unloading...");
    }

    return true;
}
