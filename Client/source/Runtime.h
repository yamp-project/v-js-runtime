#pragma once

#include <fw/utils/InstanceBase.h>
#include <v-sdk/Runtime.hpp>
#include <fw/Logger.h>
#include <functional>
#include <v8helper.h>
#include <vector>
#include <v8.h>

// TODO: move to the sdk
enum class PolymorphicalValueType : uint8_t
{
    NUMBER = 0,
    STRING = 1,
    BOOLEAN = 2
};

struct PolymorphicalValue
{
    PolymorphicalValueType m_Type;

    union
    {
        void* m_Pointer;
        char* m_String;

        double m_Number;
        bool m_Boolean;

        struct
        {
            void* m_Buffer;
            uint32_t m_Length;
        }* m_Array;
    } m_Value;

    template <typename T>
    inline T As() const
    {
        return std::move(*(T*)(&m_Value.m_Pointer));
    }
};

namespace js
{
    namespace sdk = yamp::sdk;

    class Resource;
    class Runtime : public fw::utils::InstanceBase, public sdk::IRuntimeBase
    {
    public:
        static void OnEvent(const char* eventName, PolymorphicalValue* args[], size_t size);

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

        [[nodiscard]] inline std::vector<Resource*>& GetResources()
        {
            return m_Resources;
        }

        [[nodiscard]] Resource* GetResourceByContext(v8::Local<v8::Context> context) const;
        [[nodiscard]] inline fw::Logger& Log()
        {
            return *fw::Logger::Get("JS::Runtime");
        }

        IMPLEMENT_INSTANCE_FUNCTION(Runtime);

    private:
        std::vector<Resource*> m_Resources;
        std::unique_ptr<v8::Platform> m_Platform;
        v8::Isolate* m_Isolate;

        void SetupIsolate();
    };
} // namespace js
