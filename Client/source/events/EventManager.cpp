#include "EventManager.h"

#include "../Runtime.h"
#include "../Resource.h"

namespace js
{
    void EventManager::On(v8helper::FunctionContext& ctx)
    {
        if (!ctx.CheckArgCount(2))
        {
            return;
        }

        js::Resource* resource = Runtime::GetInstance()->GetResourceByContext(ctx.GetContext());
        if (resource == nullptr)
        {
            return;
        }

        v8::Isolate* isolate = ctx.GetIsolate();
        v8::HandleScope scope(isolate);

        auto eventName = ctx.GetArg<std::string>(0);
        v8::Local<v8::Value> callbackValue;
        if (!ctx.GetArg(1, callbackValue, v8helper::Type::FUNCTION))
        {
            return;
        }

        auto callback = v8helper::Persistent<v8::Function>(isolate, v8::Local<v8::Function>::Cast(callbackValue));
        EventManager* events = resource->GetEventManager();

        if (!events->m_EventHandlers.contains(eventName))
        {
            auto handlers = std::vector<v8helper::Persistent<v8::Function>>{callback};
            events->m_EventHandlers.emplace(eventName, handlers);
        }
        else
        {
            events->m_EventHandlers.at(eventName).push_back(callback);
        }
    }

    EventManager::EventManager(Resource* resource) : m_Resource(resource)
    {
        //
    }

    EventManager::~EventManager()
    {
        delete m_Resource;
    }

    void EventManager::Fire(std::string_view eventName, std::vector<v8::Local<v8::Value>>& args)
    {
        if (!m_EventHandlers.contains(eventName.data()))
        {
            return;
        }

        v8::Isolate* isolate = m_Resource->GetIsolate();
        v8::Local<v8::Context> ctx = m_Resource->GetContext().Get(isolate);

        for (auto callback : m_EventHandlers.at(eventName.data()))
        {
            auto _ = callback.Get(isolate)->Call(ctx, ctx->Global(), args.size(), args.data());
        }
    }
} // namespace js

// built-in events from the client
// events received from another resource
