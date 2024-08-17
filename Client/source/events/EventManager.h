#pragma once

#include "common.h"
#include "events/EventManager.h"
#include "fw/Logger.h"

#include <unordered_map>
#include <optional>
#include <values.h>
#include <vector>
#include <string>

#include <v8helper.h>
#include <v8.h>

enum EventType : uint8_t
{
    CORE = 0,
    LOCAL = 1,
    REMOTE = 2
};

namespace js
{

    class Resource;
    class EventManager
    {
    public:
        using EventHandlers = std::unordered_map<std::string, std::vector<v8helper::Persistent<v8::Function>>>;

        static void OnCore(v8helper::FunctionContext& ctx);
        static void OnLocal(v8helper::FunctionContext& ctx);
        static void OnRemote(v8helper::FunctionContext& ctx);

        EventManager(Resource* resource);
        ~EventManager();

        void RegisterEvent(EventType eventType, std::string_view eventName, v8helper::Persistent<v8::Function> callback);
        void DispatchEvent(EventType eventType, std::string_view eventName, std::vector<v8::Local<v8::Value>>& args);

    private:
        Resource* m_ParentResource;

        EventHandlers m_CoreEventHandlers;
        EventHandlers m_LocalEventHandlers;
        EventHandlers m_RemoteEventHandlers;

        [[nodiscard]] inline EventHandlers* GetEventHandlersFromType(EventType eventType)
        {
            switch (eventType)
            {
            case EventType::CORE:
                return &m_CoreEventHandlers;

            case EventType::LOCAL:
                return &m_LocalEventHandlers;

            case EventType::REMOTE:
                return &m_RemoteEventHandlers;

            default:
                return nullptr;
            }
        }
    };
}; // namespace js
