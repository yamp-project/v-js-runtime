#ifndef V8_LOGFUNCTION_H
#define V8_LOGFUNCTION_H

#include "../global_template.h"
#include <v8.h>

namespace js::templates {
    class log_function final {
    public:
        static std::string Name() {
            return "yamp_log";
        }

        static void Callback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            const v8::Local<v8::Object> selfRef = args.This();
            const v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(selfRef->GetInternalField(0));
            auto logger = static_cast<Logger*>(wrap->Value());


            if (args.Length() > 0) {
                v8::String::Utf8Value utf8(args.GetIsolate(), args[0]);
                logger->Info("Log from resource: %s", *utf8);
            }
        }
    };
}


#endif //V8_LOGFUNCTION_H
