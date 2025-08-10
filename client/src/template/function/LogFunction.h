#ifndef V8_LOGFUNCTION_H
#define V8_LOGFUNCTION_H
#include "../global_template.h"

namespace js::templates {
    class LogFunction final {
    public:
        static std::string FunctionName() {
            return "yamp_log";
        }

        static void Callback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            v8::Local<v8::External> data = v8::Local<v8::External>::Cast(args.Data());
            auto* selfRef = static_cast<global::GlobalTemplate*>(data->Value());

            if (args.Length() > 0) {
                v8::String::Utf8Value utf8(args.GetIsolate(), args[0]);
                selfRef->GetLogger()->Info("Log from resource: %s", *utf8);
            }
        }
    };
}


#endif //V8_LOGFUNCTION_H
