#include "EventManager.h"

#include "Resource.h"
#include "Runtime.h"

namespace js
{
    void EventManager::OnCore(v8helper::FunctionContext& ctx)
    {
        js::Resource* resource = Runtime::GetInstance()->GetResourceByContext(ctx.GetContext());
        if (!ctx.CheckArgCount(2) || resource == nullptr)
        {
            return;
        }

        v8::Isolate* isolate = ctx.GetIsolate();
        v8::HandleScope scope(isolate);

        std::string eventName = ctx.GetArg<std::string>(0);
        v8::Local<v8::Value> callbackValue;
        if (!ctx.GetArg(1, callbackValue, v8helper::Type::FUNCTION))
        {
            return;
        }

        auto callback = v8helper::Persistent<v8::Function>(isolate, v8::Local<v8::Function>::Cast(callbackValue));
        resource->GetEventManager().RegisterEvent(EventType::CORE, eventName, callback);
    }

    EventManager::EventManager(Resource* resource) : m_ParentResource(resource), m_CoreEventHandlers(), m_LocalEventHandlers(), m_RemoteEventHandlers()
    {
        //
    }

    EventManager::~EventManager()
    {
        delete m_ParentResource;
    }

    void EventManager::RegisterEvent(EventType eventType, std::string_view eventName, v8helper::Persistent<v8::Function> callback)
    {
        auto map = GetEventHandlersFromType(eventType);
        if (map == nullptr)
        {
            return;
        }

        if (!map->contains(eventName.data()))
        {
            auto handlers = std::vector<v8helper::Persistent<v8::Function>>{callback};
            map->emplace(eventName, handlers);
        }
        else
        {
            map->at(eventName.data()).push_back(callback);
        }
    }

    void EventManager::DispatchEvent(EventType eventType, std::string_view eventName, std::vector<v8::Local<v8::Value>>& args)
    {
        auto map = GetEventHandlersFromType(eventType);
        if (map != nullptr && map->contains(eventName.data()))
        {
            v8::Isolate* isolate = m_ParentResource->GetIsolate();
            v8::Local<v8::Context> context = m_ParentResource->GetContext();

            for (auto callback : map->at(eventName.data()))
            {
                auto _ = callback.Get(isolate)->Call(context, context->Global(), args.size(), args.data());
            }
        }
    }
} // namespace js
