#pragma once

#include "v8-primitive.h"
#include <v8.h>

namespace js
{
    class StackTrace
    {
    public:
        bool IsEmpty() const
        {
            return m_Frames.empty();
        }

        std::string ToString(uint32_t offset) const
        {
            std::stringstream stream;
            size_t size = m_Frames.size();

            for (size_t i = offset; i < size; i++)
            {
                const Frame& frame = m_Frames[i];
                stream << "  at " << frame.function << " (" << frame.file << ":" << frame.line << ")"
                       << "\n";
            }

            return stream.str();
        }

        static StackTrace GetCurrent(v8::Isolate* isolate, const std::string& fileName, int framesToSkip)
        {
            v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(isolate, framesToSkip + 5);

            std::vector<Frame> frames;
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

                Frame frameData;
                frameData.file = fileName;
                frameData.line = frame->GetLineNumber();

                if (frame->GetFunctionName().IsEmpty())
                {
                    frameData.function = "[anonymous]";
                }
                else
                {
                    frameData.function = *v8::String::Utf8Value(isolate, frame->GetFunctionName());
                }

                frames.push_back(std::move(frameData));
            }

            return StackTrace{std::move(frames)};
        }

    private:
        struct Frame
        {
            std::string file;
            std::string function;
            int line;
        };

        StackTrace(const std::vector<Frame>&& frames) : m_Frames(frames){};

        std::vector<Frame> m_Frames;
    };
} // namespace js
