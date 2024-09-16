#pragma once

#include <v8helper.h>

namespace bindings::global
{
    void SetTimeout(v8helper::FunctionContext& ctx);
    void SetInterval(v8helper::FunctionContext& ctx);
    void Print(v8helper::FunctionContext& ctx);
} // namespace bindings::global
