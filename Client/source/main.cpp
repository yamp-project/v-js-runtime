#include "Runtime.h"

#include <stdint.h>
#include <v-sdk/factories/RuntimeFactory.hpp>
#include <spdlog/spdlog.h>
#include <fw/Logger.h>
#include <Windows.h>

namespace sdk = yamp::sdk;

BOOL WINAPI DllMain(HINSTANCE instanceDll, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        spdlog::set_pattern(fmt::format("[{}] [{}] {}", fw::Logger::TimePattern, fw::Logger::LogPattern, fw::Logger::MessagePattern));

        fw::Logger* logger = fw::Logger::Get("JS");
        logger->SetPattern(fmt::format("[JS] {}", fw::Logger::MessagePattern));
        logger->Info("Runtime has been loaded! {}", fmt::ptr(reserved));
        logger->Info("Author: yamp");
        logger->Info("Version: 0.0.1-alpha");

        auto runtimeFactory = sdk::IRuntimeFactory::GetInstance();
        logger->Info("Runtime factory instance: {}\n", fmt::ptr(runtimeFactory));

        js::Runtime* runtime = js::Runtime::GetInstance();

        // TODO: terminate the process if sdk::Return is a falsy value
        sdk::Result result = runtimeFactory->RegisterRuntime(runtime);
        result = runtimeFactory->RegisterRuntimeResourceHandling("js", runtime);

        // TODO: add a state to the runtime to know if v8 was succesfuly initialized
        // TODO: should be called from the sdk
        runtime->OnStart();
    }

    if (reason == DLL_PROCESS_DETACH)
    {
        // TODO: handle dll unloading
        fw::Logger::Get("JS")->Info("Runtime is unloading...");
    }

    return true;
}
