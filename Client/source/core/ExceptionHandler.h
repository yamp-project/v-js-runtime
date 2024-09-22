#pragma once

#include "helpers/js/SourceLocation.h"
#include "helpers/js/StackTrace.h"

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
            js::SourceLocation m_SourceLocation;
            js::StackTrace m_StackTrace;

            // clang-format off
            PromiseRejection(
                std::unique_ptr<v8::Persistent<v8::Promise>> promise,
                std::unique_ptr<v8::Persistent<v8::Value>> value,
                js::SourceLocation sourceLocation,
                js::StackTrace stackTrace
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

        ExceptionHandler(Resource* parentResource);

        void ProcessExceptions();

    private:
        Resource* m_ParentResource;

        std::vector<PromiseRejection> m_PromiseRejections;

        void OnPromiseRejectedWithoutHandler(const v8::PromiseRejectMessage& message);
        void OnPromiseHandlerAdded(const v8::PromiseRejectMessage& message);
        void OnPromiseRejectAfterResolve(v8::PromiseRejectMessage& message);
        void OnPromiseResolveAfterResolve(v8::PromiseRejectMessage& message);
    };
} // namespace core
