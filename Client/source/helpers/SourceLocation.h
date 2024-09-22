#pragma once

#include "Resource.h"
#include <v8.h>

struct SourceLocation
{
    int m_Line = 0;
    bool m_Valid = false;

    std::string m_File;

    SourceLocation() = default;
    SourceLocation(const std::string& file, int line) : m_Valid(true), m_File(file), m_Line(line)
    {
        //
    }

    static SourceLocation GetCurrent(js::Resource* resource, int framesToSkip = 0)
    {
        v8::Isolate* isolate = resource->GetIsolate();
        v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(isolate, framesToSkip + 5);

        for (int i = framesToSkip; i < stackTrace->GetFrameCount(); i++)
        {
            v8::Local<v8::StackFrame> frame = stackTrace->GetFrame(isolate, i);
            if (!frame->IsUserJavaScript())
            {
                continue;
            }

            auto localScriptName = frame->GetScriptName();
            if (localScriptName.IsEmpty())
            {
                continue;
            }

            std::string scriptName = *v8::String::Utf8Value(isolate, localScriptName);
            if (scriptName.empty())
            {
                continue;
            }

            return SourceLocation{resource->GetResourceInformation()->m_MainFile, frame->GetLineNumber()};
        }

        return SourceLocation{};
    }
};
