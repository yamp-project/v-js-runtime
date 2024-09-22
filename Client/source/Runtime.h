#pragma once

#include "stdafx.h"

#include <fw/utils/InstanceBase.h>
#include <fw/Logger.h>

#include <v-sdk/Runtime.hpp>
#include <v-sdk/Events.hpp>
#include <v8helper.h>

namespace js
{
    namespace sdk = yamp::sdk;

    class Resource;
    class Runtime : public fw::utils::InstanceBase, public sdk::IRuntimeBase
    {
    public:
        [[nodiscard]] inline static fw::Logger& Log()
        {
            return *fw::Logger::Get("JS::Runtime");
        }

        static void OnEvent(yamp::sdk::AnyBuiltinEvent* ev);
        static size_t OnNearHeapLimit(void*, size_t current, size_t initial);
        static void OnHeapOOM(const char* location, bool isHeap);
        static void OnFatalError(const char* location, const char* message);
        static void OnPromiseRejected(v8::PromiseRejectMessage message);

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

        Resource* GetResourceByContext(const v8::Local<v8::Context>& context) const;
        inline std::vector<Resource*>& GetResources()
        {
            return m_Resources;
        }

        inline v8::Isolate* GetIsolate() const
        {
            return m_Isolate;
        }

        IMPLEMENT_INSTANCE_FUNCTION(Runtime);

    private:
        std::unique_ptr<v8::Platform> m_Platform;
        std::vector<Resource*> m_Resources;
        v8::Isolate* m_Isolate;

        void SetupIsolate();
    };
} // namespace js
