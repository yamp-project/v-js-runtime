#pragma once

#include "stdafx.h"

#include <fw/Logger.h>
#include <v8helper.h>

namespace js
{
    class Resource;
}

namespace core
{
    enum EventType : uint8_t
    {
        CORE = 0,
        LOCAL,
        REMOTE
    };

    class EventManager
    {
    public:
        using EventHandlers = std::unordered_map<std::string, std::vector<v8helper::Persistent<v8::Function>>>;

        static void OnCore(v8helper::FunctionContext& ctx);
        static void OnLocal(v8helper::FunctionContext& ctx);
        static void OnRemote(v8helper::FunctionContext& ctx);

        EventManager(js::Resource* parentResource);

        void RegisterEvent(EventType eventType, std::string_view eventName, v8helper::Persistent<v8::Function> callback);
        void DispatchEvent(EventType eventType, std::string_view eventName, std::vector<v8::Local<v8::Value>>& args);

    private:
        js::Resource* m_ParentResource;

        EventHandlers m_CoreEventHandlers;
        EventHandlers m_LocalEventHandlers;
        EventHandlers m_RemoteEventHandlers;

        inline EventHandlers* GetEventHandlersFromType(EventType eventType);
    };
}; // namespace core
