#pragma once

#include "helpers/SourceLocation.h"
#include "helpers/StackTrace.h"

#include <v8.h>

namespace core
{
    class ExceptionHandler
    {
    public:
        struct PromiseRejection
        {
            std::unique_ptr<v8::Persistent<v8::Promise>> m_Promise;
            std::shared_ptr<v8::Persistent<v8::Value>> m_Value;
            SourceLocation m_SourceLocation;
            StackTrace m_StackTrace;

            // clang-format off
            PromiseRejection(
                std::unique_ptr<v8::Persistent<v8::Promise>> promise,
                std::unique_ptr<v8::Persistent<v8::Value>> value,
                SourceLocation sourceLocation,
                StackTrace stackTrace
            )
                : m_Promise(std::move(promise)), m_Value(std::move(value)), m_SourceLocation(std::move(sourceLocation)), m_StackTrace(stackTrace)
            {
                // clang-format on
            }

            std::string GetRejectionMessage() const
            {
                return "test";
            }
        };

        static void OnPromiseRejected(v8::PromiseRejectMessage message);

        ExceptionHandler(js::Resource* parentResource);

        void ProcessExceptions();

    private:
        js::Resource* m_ParentResource;

        std::vector<PromiseRejection> m_PromiseRejections;

        void OnPromiseRejectedWithoutHandler(const v8::PromiseRejectMessage& message);
        void OnPromiseHandlerAdded(const v8::PromiseRejectMessage& message);
        void OnPromiseRejectAfterResolve(v8::PromiseRejectMessage& message);
        void OnPromiseResolveAfterResolve(v8::PromiseRejectMessage& message);
    };
} // namespace core
