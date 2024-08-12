#pragma once

#include "fw/Logger.h"

#include <unordered_map>
#include <optional>
#include <values.h>
#include <vector>
#include <string>

#include <v8helper.h>
#include <v8.h>

namespace js
{
    class Resource;
    class EventManager
    {
    public:
        static void On(v8helper::FunctionContext& ctx);

        EventManager(Resource* resource);
        ~EventManager();

        void Fire(std::string_view eventName);

        [[nodiscard]] inline Resource* GetParentResource() const
        {
            return m_Resource;
        }

        // TODO: change visibility to private and expose methods
        std::unordered_map<std::string, std::vector<v8helper::Persistent<v8::Function>>> m_EventHandlers;

    private:
        Resource* m_Resource;
    };
}; // namespace js
