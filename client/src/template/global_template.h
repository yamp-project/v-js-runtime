#ifndef V8_GLOBAL_TEMPLATE_H
#define V8_GLOBAL_TEMPLATE_H
#include "v8.h"
#include "../core/resource.h"
#include "util/logger.h"

namespace js::templates {
    class GlobalTemplate {
    public:
        GlobalTemplate(v8::Isolate* isolate, Logger* logger, Resource* resource);
        ~GlobalTemplate();

        v8::Local<v8::ObjectTemplate> GetGlobalTemplate();

        Logger* GetLogger() const {
            return m_Logger;
        }
    private:
        v8::Local<v8::ObjectTemplate> m_Template;
        v8::Isolate* m_Isolate;

        Resource* m_Resource;
        Logger* m_Logger;
    };
}

#endif //V8_GLOBAL_TEMPLATE_H
