#include "ExceptionHandler.h"

#include "Resource.h"
#include "Runtime.h"

#include <fw/Logger.h>

namespace core
{
    void ExceptionHandler::OnPromiseRejected(v8::PromiseRejectMessage message)
    {
        v8::Local<v8::Context> context = message.GetPromise()->GetCreationContextChecked();
        if (Resource* resource = Runtime::GetInstance()->GetResourceByContext(context))
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

    ExceptionHandler::ExceptionHandler(Resource* parentResource) : m_PromiseRejections(), m_ParentResource(parentResource)
    {
        //
    }

    void ExceptionHandler::ProcessExceptions()
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

    void ExceptionHandler::OnPromiseRejectedWithoutHandler(const v8::PromiseRejectMessage& message)
    {
        v8::Isolate* isolate = m_ParentResource->GetIsolate();

        // clang-format off
            m_PromiseRejections.emplace_back(
                std::make_unique<v8::Persistent<v8::Promise>>(isolate, message.GetPromise()),
                std::make_unique<v8::Persistent<v8::Value>>(isolate, message.GetValue()),
                js::SourceLocation::GetCurrent(m_ParentResource),
                js::StackTrace::GetCurrent(isolate, m_ParentResource->GetResourceInformation()->m_MainFile, 0)
            );
        // clang-format on
    }

    void ExceptionHandler::OnPromiseHandlerAdded(const v8::PromiseRejectMessage& message)
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

    void ExceptionHandler::OnPromiseRejectAfterResolve(v8::PromiseRejectMessage& message)
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

    void ExceptionHandler::OnPromiseResolveAfterResolve(v8::PromiseRejectMessage& message)
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
}; // namespace core
