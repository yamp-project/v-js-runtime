#ifndef V8_LOGCLASS_H
#define V8_LOGCLASS_H
#include <v8.h>
#include "util/logger.h"
#include "util/utils.h"

namespace js::templates {
    class LogClass final {
    public:
        static std::string Name() {
            return "yamp_logger";
        }

        static v8::Local<v8::ObjectTemplate> CreateTemplate(v8::Isolate* isolate) {
            v8::EscapableHandleScope escapableHandleScope(isolate);
            v8::Local<v8::ObjectTemplate> oTemplate = v8::ObjectTemplate::New(isolate);

            oTemplate->SetInternalFieldCount(1);

            oTemplate->Set(
                utils::StringToV8(isolate, "info"),
                v8::FunctionTemplate::New(isolate, InfoCallback)
            );

            oTemplate->Set(
                utils::StringToV8(isolate, "error"),
                v8::FunctionTemplate::New(isolate, ErrorCallback)
            );
        };

    private:
        static void InfoCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            const v8::Local<v8::Object> selfRef = args.This();
            const v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(selfRef->GetInternalField(0));
            auto logger = static_cast<Logger*>(wrap->Value());

            if (args.Length() > 0) {
                v8::String::Utf8Value utf8(args.GetIsolate(), args[0]);
                logger->Info(*utf8);
            }
        }

        static void ErrorCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            const v8::Local<v8::Object> selfRef = args.This();
            const v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(selfRef->GetInternalField(0));
            auto logger = static_cast<Logger*>(wrap->Value());

            if (args.Length() > 0) {
                v8::String::Utf8Value utf8(args.GetIsolate(), args[0]);
                logger->Error(*utf8);
            }
        }
    };
}


#endif //V8_LOGCLASS_H