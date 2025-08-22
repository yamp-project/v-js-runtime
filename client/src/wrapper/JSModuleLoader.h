#ifndef V_JS_RUNTIME_JSMODULELOADER_H
#define V_JS_RUNTIME_JSMODULELOADER_H
#include <filesystem>
#include <unordered_map>
#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>

#include "util/logger.h"
#include "util/utils.h"

namespace js::wrapper {
    class JSModuleLoader {
    public:
        JSModuleLoader(const JSContextRef context, const std::filesystem::path& path, Logger* logger) : m_Context(context), m_ResourcePath(path), m_Logger(logger) {
            SetupRequireFunction();
        }

        bool LoadMainFile(const std::filesystem::path& mainFile) {
            try {
                std::string code = utils::ReadFile(mainFile.string());
                if (code.empty()) {
                    return false;
                }

                std::filesystem::path currentDir = mainFile.parent_path();

                return ExecuteScript(code, mainFile.string());
            } catch (const std::exception& e) {
                return false;
            }
        }
    private:
        bool ExecuteScript(const std::string& code, const std::string& filename = "") {
            const JSStringRef script = JSStringCreateWithUTF8CString(code.c_str());
            JSStringRef sourceUrl = nullptr;

            if (!filename.empty()) {
                sourceUrl = JSStringCreateWithUTF8CString(filename.c_str());
            }

            JSValueRef exception = nullptr;
            JSEvaluateScript(
                m_Context,
                script,
                nullptr,
                sourceUrl,
                1,
                &exception);

            JSStringRelease(script);
            if (sourceUrl) {
                JSStringRelease(sourceUrl);
            }

            if (exception) {
                HandleException(exception);
                return false;
            }

            return true;
        }

        void SetupRequireFunction() {
            JSStringRef requireName = JSStringCreateWithUTF8CString("require");

            const JSObjectRef requireFunction = JSObjectMakeFunctionWithCallback(
                m_Context,
                requireName,
                RequireCallback);

            const JSStringRef thisPointerKey = JSStringCreateWithUTF8CString("__moduleLoader");
            const JSValueRef thisPointerValue = JSValueMakeNumber(m_Context, reinterpret_cast<uintptr_t>(this));
            JSObjectSetProperty(m_Context, requireFunction, thisPointerKey, thisPointerValue,
                                      kJSPropertyAttributeDontEnum, nullptr);

            JSObjectRef globalObject = JSContextGetGlobalObject(m_Context);
            JSObjectSetProperty(m_Context, globalObject, requireName, requireFunction, kJSPropertyAttributeNone, nullptr);

            JSStringRelease(requireName);
            JSStringRelease(thisPointerKey);
        }

        static JSValueRef RequireCallback(const JSContextRef context, const JSObjectRef function, JSObjectRef thisObject, const size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception) {
            if (argumentCount < 1) {
                *exception = CreateError(context, "require() expects a module path");
                return JSValueMakeUndefined(context);
            }

            const JSStringRef thisPointerKey = JSStringCreateWithUTF8CString("__moduleLoader");
            JSValueRef thisPointerValue = JSObjectGetProperty(context, function, thisPointerKey, nullptr);
            JSStringRelease(thisPointerKey);

            auto* loader = reinterpret_cast<JSModuleLoader*>(static_cast<uintptr_t>(JSValueToNumber(context, thisPointerValue, nullptr)));

            JSStringRef modulePath = JSValueToStringCopy(context, arguments[0], exception);
            if (*exception) {
                return JSValueMakeUndefined(context);
            }

            const size_t bufferSize = JSStringGetMaximumUTF8CStringSize(modulePath);
            const auto buffer = new char[bufferSize];
            JSStringGetUTF8CString(modulePath, buffer, bufferSize);
            const std::string modulePathStr(buffer);
            delete[] buffer;
            JSStringRelease(modulePath);

            const JSValueRef moduleExports = loader->LoadModule(modulePathStr);
            return moduleExports;
        }

        JSValueRef LoadModule(const std::string& modulePath) {
            auto it = m_ModuleCache.find(modulePath);
            if (it != m_ModuleCache.end()) {
                return it->second;
            }

            std::filesystem::path fullPath = ResolveModulePath(modulePath);
            if (fullPath.empty()) {
                return JSValueMakeUndefined(m_Context);
            }

            try {
                std::string moduleCode = utils::ReadFile(fullPath.string());

                JSObjectRef moduleObj = JSObjectMake(m_Context, nullptr, nullptr);
                JSObjectRef exportObj = JSObjectMake(m_Context, nullptr, nullptr);

                JSStringRef exportKey = JSStringCreateWithUTF8CString("exports");
                JSObjectSetProperty(m_Context, moduleObj, exportKey, exportObj, kJSPropertyAttributeNone, nullptr);

                JSObjectRef globalObj = JSContextGetGlobalObject(m_Context);
                JSStringRef moduleKey = JSStringCreateWithUTF8CString("module");
                JSObjectSetProperty(m_Context, globalObj, moduleKey, moduleObj, kJSPropertyAttributeNone, nullptr);
                JSObjectSetProperty(m_Context, globalObj, exportKey, exportObj, kJSPropertyAttributeNone, nullptr);

                ExecuteScript(moduleCode, fullPath.string());

                JSValueRef finalExport = JSObjectGetProperty(m_Context, moduleObj, exportKey, nullptr);

                if (JSValueIsObject(m_Context, finalExport)) {
                    m_ModuleCache[modulePath] = JSValueToObject(m_Context, finalExport, nullptr);
                }

                JSObjectDeleteProperty(m_Context, globalObj, moduleKey, nullptr);
                JSObjectDeleteProperty(m_Context, globalObj, exportKey, nullptr);

                JSStringRelease(exportKey);
                JSStringRelease(moduleKey);

                return finalExport;
            } catch (const std::exception& e) {
                return JSValueMakeUndefined(m_Context);
            }
        }

        std::filesystem::path ResolveModulePath(const std::string& modulePath) const {
            if (modulePath.starts_with("../")) {
                // invalid escaping
                CreateError(m_Context, "Invalid path (" + modulePath + ")");
                return "";
            }

            const std::filesystem::path resolved = m_ResourcePath / modulePath;

            std::vector<std::string> extensions = {"", ".js", ".json"};

            for (const auto& extension : extensions) {
                std::filesystem::path candidate = resolved;
                candidate.append(extension);

                if (std::filesystem::exists(candidate) && std::filesystem::is_regular_file(candidate)) {
                    return candidate;
                }
            }

            return {};
        }

        static JSValueRef CreateError(const JSContextRef context, const std::string& message) {
            const JSStringRef errorStr = JSStringCreateWithUTF8CString(message.c_str());
            const JSValueRef args[] = { JSValueMakeString(context, errorStr) };
            JSStringRelease(errorStr);

            const JSStringRef errorConstructor = JSStringCreateWithUTF8CString("Error");
            const JSObjectRef globalObj = JSContextGetGlobalObject(context);
            const JSValueRef errorClass = JSObjectGetProperty(context, globalObj, errorConstructor, nullptr);
            JSStringRelease(errorConstructor);

            if (JSValueIsObject(context, errorClass)) {
                const JSObjectRef errorClassObj = JSValueToObject(context, errorClass, nullptr);
                return JSObjectCallAsConstructor(context, errorClassObj, 1, args, nullptr);
            }

            return JSValueMakeUndefined(context);
        }

        void HandleException(const JSValueRef exception) {
            if (!exception) {
                return;
            }

            const JSStringRef exceptionStr = JSValueToStringCopy(m_Context, exception, nullptr);
            if (exceptionStr) {
                const size_t bufferSize = JSStringGetMaximumUTF8CStringSize(exceptionStr);
                auto* buffer = new char[bufferSize];
                JSStringGetUTF8CString(exceptionStr, buffer, bufferSize);

                m_Logger->Error("JavaScript Error: %s\n", buffer);

                delete[] buffer;
                JSStringRelease(exceptionStr);
            }
        }
    private:
        JSContextRef m_Context;
        std::filesystem::path m_ResourcePath;
        std::unordered_map<std::string, JSObjectRef> m_ModuleCache;
        Logger *m_Logger;
    };
}


#endif //V_JS_RUNTIME_JSMODULELOADER_H