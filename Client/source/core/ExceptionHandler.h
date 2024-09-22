#pragma once

#include "Runtime.h"
#include "Resource.h"
#include "helpers/SourceLocation.h"
#include "helpers/StackTrace.h"

#include <fw/Logger.h>
#include <memory>
#include <string>
#include <vector>
#include <v8.h>

namespace js
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

        static void OnPromiseRejected(v8::PromiseRejectMessage message)
        {
            v8::Local<v8::Context> context = message.GetPromise()->GetCreationContextChecked();
            if (js::Resource* resource = js::Runtime::GetInstance()->GetResourceByContext(context))
            {
                ExceptionHandler& handler = resource->GetExceptionHandler();
                switch (message.GetEvent())
                {
                case v8::kPromiseRejectWithNoHandler:
                    handler.OnPromiseRejectedWithoutHandler(message);
                    break;
                case v8::kPromiseHandlerAddedAfterReject:
                    handler.OnPromiseHandlerAdded(message);
                    break;
                case v8::kPromiseRejectAfterResolved:
                    handler.OnPromiseRejectAfterResolve(message);
                    break;
                case v8::kPromiseResolveAfterResolved:
                    handler.OnPromiseResolveAfterResolve(message);
                    break;
                default:
                    break;
                }
            }
        }

        ExceptionHandler(js::Resource* resource) : m_PromiseRejections(), m_ParentResource(resource)
        {
            //
        }

        void ProcessExceptions()
        {
            v8::Isolate* isolate = m_ParentResource->GetIsolate();
            v8::Local<v8::Context> context = m_ParentResource->GetContext();
            const char* resourceName = m_ParentResource->GetResourceInformation()->m_Name;

            for (const PromiseRejection& rejection : m_PromiseRejections)
            {
                std::string rejectionMsg = *v8::String::Utf8Value(isolate, rejection.m_Value->Get(isolate)->ToString(context).ToLocalChecked());
                fw::Logger::Get("DEBUG")->Error("Unhandled promise rejection in resource {} in file {} at line {}", resourceName, rejection.m_SourceLocation.m_File, rejection.m_SourceLocation.m_Line);

                if (!rejectionMsg.empty())
                {
                    fw::Logger::Get("DEBUG")->Error("{}", rejectionMsg);
                }

                if (!rejection.m_StackTrace.IsEmpty())
                {
                    fw::Logger::Get("DEBUG")->Error("{}", rejection.m_StackTrace.ToString(0));
                }
            }

            m_PromiseRejections.clear();
        }

    private:
        void OnPromiseRejectedWithoutHandler(const v8::PromiseRejectMessage& message)
        {
            v8::Isolate* isolate = m_ParentResource->GetIsolate();

            // clang-format off
            m_PromiseRejections.emplace_back(
                std::make_unique<v8::Persistent<v8::Promise>>(isolate, message.GetPromise()),
                std::make_unique<v8::Persistent<v8::Value>>(isolate, message.GetValue()),
                SourceLocation::GetCurrent(m_ParentResource),
                StackTrace::GetCurrent(isolate, m_ParentResource->GetResourceInformation()->m_MainFile, 0)
            );
            // clang-format on
        }

        void OnPromiseHandlerAdded(const v8::PromiseRejectMessage& message)
        {
            // clang-format off
            v8::Isolate* isolate = m_ParentResource->GetIsolate();
            auto newEnd = std::remove_if(m_PromiseRejections.begin(), m_PromiseRejections.end(), [&](PromiseRejection& rejection)
            {
                return rejection.m_Promise->Get(isolate) == message.GetPromise();
            });
            // clang-format on

            m_PromiseRejections.erase(newEnd, m_PromiseRejections.end());
        }

        void OnPromiseRejectAfterResolve(v8::PromiseRejectMessage& message)
        {
            const char* resourceName = m_ParentResource->GetResourceInformation()->m_Name;
            fw::Logger::Get("DEBUG")->Error("Promise rejected after already being resolved in resource {}", resourceName);

            v8::Isolate* isolate = m_ParentResource->GetIsolate();
            std::string rejectionMsg = *v8::String::Utf8Value(isolate, message.GetValue()->ToString(m_ParentResource->GetContext()).ToLocalChecked());
            if (!rejectionMsg.empty())
            {
                fw::Logger::Get("DEBUG")->Error("{}", rejectionMsg);
            }
        }

        void OnPromiseResolveAfterResolve(v8::PromiseRejectMessage& message)
        {
            const char* resourceName = m_ParentResource->GetResourceInformation()->m_Name;
            fw::Logger::Get("DEBUG")->Error("Promise resolved after already being resolved in resource {}", resourceName);

            v8::Isolate* isolate = m_ParentResource->GetIsolate();
            std::string rejectionMsg = *v8::String::Utf8Value(isolate, message.GetValue()->ToString(m_ParentResource->GetContext()).ToLocalChecked());
            if (!rejectionMsg.empty())
            {
                fw::Logger::Get("DEBUG")->Error("{}", rejectionMsg);
            }
        }

        std::vector<PromiseRejection> m_PromiseRejections;
        Resource* m_ParentResource;
    };
} // namespace js
