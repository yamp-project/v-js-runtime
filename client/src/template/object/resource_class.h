#ifndef V8_LOGCLASS_H
#define V8_LOGCLASS_H
#include <v8.h>
#include "util/utils.h"

namespace js::templates {
    class resource_class final {
    public:
        static std::string Name() {
            return "resource";
        }

        static v8::Local<v8::ObjectTemplate> CreateTemplate(v8::Isolate* isolate) {
            v8::EscapableHandleScope escapableHandleScope(isolate);
            const v8::Local<v8::ObjectTemplate> oTemplate = v8::ObjectTemplate::New(isolate);

            oTemplate->SetInternalFieldCount(1);

            // yamp.resource.name();
            oTemplate->Set(
                utils::StringToV8(isolate, "name"),
                v8::FunctionTemplate::New(isolate, NameCallback)
            );

            // yamp.resource.resourcePath();
            oTemplate->Set(
                utils::StringToV8(isolate, "resourcePath"),
                v8::FunctionTemplate::New(isolate, ResourcePathCallback)
            );

            // yamp.resource.resourceMainFile();
        	oTemplate->Set(
                utils::StringToV8(isolate, "resourceMainFile"),
                v8::FunctionTemplate::New(isolate, ResourceMainFileCallback)
            );

            // yamp.resource.on(EventName: string, CallbackFunction: void);
        	oTemplate->Set(
                utils::StringToV8(isolate, "on"),
                v8::FunctionTemplate::New(isolate, OnEventCallback)
            );

            return oTemplate;
        };

    private:
        static void NameCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            v8::Isolate* isolate = args.GetIsolate();

            const v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
            const auto resource = static_cast<Resource*>(wrap->Value());

            args.GetReturnValue().Set(utils::StringToV8(isolate, resource->GetResourceName()));
        }

        static void ResourcePathCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            v8::Isolate* isolate = args.GetIsolate();

            const v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
            const auto resource = static_cast<Resource*>(wrap->Value());

            args.GetReturnValue().Set(utils::StringToV8(isolate, resource->GetResourcePath()));
        }

        static void ResourceMainFileCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            v8::Isolate* isolate = args.GetIsolate();

            const v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
            const auto resource = static_cast<Resource*>(wrap->Value());

            args.GetReturnValue().Set(utils::StringToV8(isolate, resource->GetMainFilePath()));
        }

        static void OnEventCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
            v8::Isolate* isolate = args.GetIsolate();
            v8::Local<v8::Value> eventNameArg = args[0];
            v8::Local<v8::Value> eventFunctionArg = args[1];

            const v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
            const auto resource = static_cast<Resource*>(wrap->Value());

            if (!eventNameArg->IsString()) {
                return;
            }

            const std::string eventName(*v8::String::Utf8Value(isolate, eventNameArg));

            if (!eventFunctionArg->IsFunction()) {
                return;
            }

            const v8::Local<v8::Function> eventFunctionRef = v8::Local<v8::Function>::Cast(eventFunctionArg);

            resource->AddEventCallback(eventName, eventFunctionRef);
        }
    };
}


#endif //V8_LOGCLASS_H