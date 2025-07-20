#ifndef RESOURCE_H
#define RESOURCE_H

#include "util/logger.h"

#include <yamp-sdk/sdk.h>
#include <v8.h>

namespace js {
    class Resource {
    public:
        void OnStart();
        void OnStop();
        void OnTick();
        //void OnEvent(CoreEvent event);

        Resource(ILookupTable* lookupTable, IResource* resource, v8::Isolate* isolate);
        ~Resource() = default;
    private:
        IResource* m_Resource;
        Logger m_Logger;

        v8::Isolate* m_Isolate;
    };
} // js

#endif //RESOURCE_H