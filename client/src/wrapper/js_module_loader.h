#ifndef V_JS_RUNTIME_JSMODULELOADER_H
#define V_JS_RUNTIME_JSMODULELOADER_H
#include <filesystem>
#include <unordered_map>
#include <regex>
#include <vector>
#include <JavaScriptCore/JSContextRef.h>
#include <JavaScriptCore/JSStringRef.h>

#include "util/logger.h"
#include "util/utils.h"

namespace js::wrapper {
    class JSModuleLoader {
    public:
        JSModuleLoader(const JSContextRef context, const std::filesystem::path& path, Logger* logger)
            : m_Context(context), m_ResourcePath(path), m_Logger(logger) {
            SetupModuleSystem();
        }

        bool LoadMainFile(const std::filesystem::path& mainFile) {
            try {
                std::string code = utils::ReadFile(mainFile.string());
                if (code.empty()) {
                    m_Logger->Error("Failed to read main file or file is empty: %s", mainFile.string().c_str());
                    return false;
                }

                code = TransformImports(code);

                return ExecuteScript(code, mainFile.string());
            } catch (const std::exception& e) {
                m_Logger->Error("Exception loading main file %s: %s", mainFile.string().c_str(), e.what());
                return false;
            }
        }

    private:
        bool ExecuteScript(const std::string& code, const std::string& filename = "") const {
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

        void SetupModuleSystem() {
            const JSStringRef importName = JSStringCreateWithUTF8CString("__import");

            const JSObjectRef importFunction = JSObjectMakeFunctionWithCallback(
                m_Context,
                importName,
                ImportCallback);

            const JSStringRef thisPointerKey = JSStringCreateWithUTF8CString("__moduleLoader");
            const JSValueRef thisPointerValue = JSValueMakeNumber(m_Context, reinterpret_cast<uintptr_t>(this));
            JSObjectSetProperty(m_Context, importFunction, thisPointerKey, thisPointerValue,
                                      kJSPropertyAttributeDontEnum, nullptr);

            JSObjectRef globalObject = JSContextGetGlobalObject(m_Context);
            JSObjectSetProperty(m_Context, globalObject, importName, importFunction, kJSPropertyAttributeNone, nullptr);

            JSStringRelease(importName);
            JSStringRelease(thisPointerKey);
        }

        std::string TransformImports(const std::string& code) {
            std::string transformed = code;

            // import { x, y } from 'module' -> const { x, y } = __import('module')
            const std::regex namedImportRegex(R"(import\s*\{\s*([^}]+)\s*\}\s*from\s*['"`]([^'"`]+)['"`]\s*;?)");
            transformed = std::regex_replace(transformed, namedImportRegex, "const { $1 } = __import('$2');");

            // import x from 'module' -> const x = __import('module').default
            const std::regex defaultImportRegex(R"(import\s+([a-zA-Z_$][a-zA-Z0-9_$]*)\s+from\s*['"`]([^'"`]+)['"`]\s*;?)");
            transformed = std::regex_replace(transformed, defaultImportRegex, "const $1 = __import('$2').default;");

            // import * as x from 'module' -> const x = __import('module')
            const std::regex namespaceImportRegex(R"(import\s*\*\s*as\s+([a-zA-Z_$][a-zA-Z0-9_$]*)\s+from\s*['"`]([^'"`]+)['"`]\s*;?)");
            transformed = std::regex_replace(transformed, namespaceImportRegex, "const $1 = __import('$2');");

            // import 'module' -> __import('module')
            const std::regex sideEffectImportRegex(R"(import\s*['"`]([^'"`]+)['"`]\s*;?)");
            transformed = std::regex_replace(transformed, sideEffectImportRegex, "__import('$1');");

            return transformed;
        }

        static JSValueRef ImportCallback(const JSContextRef context, const JSObjectRef function, JSObjectRef thisObject,
                                       const size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception) {
            if (argumentCount < 1) {
                *exception = CreateError(context, "__import() expects a module path");
                return JSValueMakeUndefined(context);
            }

            const JSStringRef thisPointerKey = JSStringCreateWithUTF8CString("__moduleLoader");
            const JSValueRef thisPointerValue = JSObjectGetProperty(context, function, thisPointerKey, nullptr);
            JSStringRelease(thisPointerKey);

            auto* loader = reinterpret_cast<JSModuleLoader*>(static_cast<uintptr_t>(JSValueToNumber(context, thisPointerValue, nullptr)));

            const JSStringRef modulePath = JSValueToStringCopy(context, arguments[0], exception);
            if (*exception) {
                return JSValueMakeUndefined(context);
            }

            const size_t bufferSize = JSStringGetMaximumUTF8CStringSize(modulePath);
            std::vector<char> buffer(bufferSize);
            JSStringGetUTF8CString(modulePath, buffer.data(), bufferSize);
            const std::string modulePathStr(buffer.data());
            JSStringRelease(modulePath);

            const JSValueRef moduleExports = loader->LoadModule(modulePathStr);
            return moduleExports;
        }

        JSValueRef LoadModule(const std::string& modulePath) {
            auto it = m_ModuleCache.find(modulePath);
            if (it != m_ModuleCache.end()) {
                return it->second;
            }

            const std::filesystem::path fullPath = ResolveModulePath(modulePath);
            if (fullPath.empty()) {
                m_Logger->Error("Module not found: %s", modulePath.c_str());
                return JSValueMakeUndefined(m_Context);
            }

            try {
                std::string moduleCode = utils::ReadFile(fullPath.string());
                if (moduleCode.empty()) {
                    m_Logger->Error("Failed to read module file or file is empty: %s", fullPath.string().c_str());
                    return JSValueMakeUndefined(m_Context);
                }

                moduleCode = TransformImports(moduleCode);

                const JSObjectRef moduleExports = JSObjectMake(m_Context, nullptr, nullptr);

                const JSObjectRef globalObj = JSContextGetGlobalObject(m_Context);

                const std::string wrappedCode = CreateModuleWrapper(moduleCode, modulePath);

                m_ModuleCache[modulePath] = moduleExports;

                const JSStringRef exportsKey = JSStringCreateWithUTF8CString("__moduleExports");
                JSObjectSetProperty(m_Context, globalObj, exportsKey, moduleExports, kJSPropertyAttributeNone, nullptr);

                const bool success = ExecuteScript(wrappedCode, fullPath.string());

                JSObjectDeleteProperty(m_Context, globalObj, exportsKey, nullptr);
                JSStringRelease(exportsKey);

                if (!success) {
                    m_ModuleCache.erase(modulePath);
                    return JSValueMakeUndefined(m_Context);
                }

                return moduleExports;
            } catch (const std::exception& e) {
                m_Logger->Error("Exception loading module %s: %s", modulePath.c_str(), e.what());
                m_ModuleCache.erase(modulePath);
                return JSValueMakeUndefined(m_Context);
            }
        }

        std::string CreateModuleWrapper(const std::string& moduleCode, const std::string& modulePath) {
            std::string wrapper = "(function() {\n";
            wrapper += "  const exports = __moduleExports;\n";
            wrapper += "  const __export = (name, value) => { exports[name] = value; };\n";
            wrapper += "  const __exportDefault = (value) => { exports.default = value; };\n";
            wrapper += "\n";

            const std::string transformedCode = TransformExports(moduleCode);

            wrapper += transformedCode;
            wrapper += "\n})();";

            return wrapper;
        }

        std::string TransformExports(const std::string& code) {
            std::string transformed = code;

            // export const/let/var x = ... -> const/let/var x = ...; __export('x', x);
            std::regex exportVarRegex(R"(export\s+(const|let|var)\s+([a-zA-Z_$][a-zA-Z0-9_$]*)\s*=\s*([^;]+);?)");
            transformed = std::regex_replace(transformed, exportVarRegex, "$1 $2 = $3; __export('$2', $2);");

            // export function name() { ... } -> function name() { ... } __export('name', name);
            std::regex exportFunctionRegex(R"(export\s+function\s+([a-zA-Z_$][a-zA-Z0-9_$]*))");

            std::sregex_iterator iter(transformed.begin(), transformed.end(), exportFunctionRegex);
            std::sregex_iterator end;

            std::vector<std::pair<size_t, std::pair<size_t, std::string>>> replacements;

            for (auto i = iter; i != end; ++i) {
                const std::smatch& match = *i;
                std::string functionName = match[1].str();
                size_t startPos = match.position();
                size_t matchLen = match.length();

                size_t searchStart = startPos + matchLen;
                size_t bracePos = transformed.find('{', searchStart);

                if (bracePos != std::string::npos) {
                    int braceCount = 1;
                    size_t endPos = bracePos + 1;

                    while (endPos < transformed.length() && braceCount > 0) {
                        if (transformed[endPos] == '{') braceCount++;
                        else if (transformed[endPos] == '}') braceCount--;
                        endPos++;
                    }

                    if (braceCount == 0) {
                        std::string replacement = "function " + functionName;
                        std::string exportCall = "\n__export('" + functionName + "', " + functionName + ");";

                        replacements.push_back({startPos, {matchLen, replacement}});
                        replacements.push_back({endPos, {0, exportCall}});
                    }
                }
            }

            std::sort(replacements.rbegin(), replacements.rend());
            for (const auto&[first, second] : replacements) {
                transformed.replace(first, second.first, second.second);
            }

            // export class Name { ... } -> class Name { ... } __export('Name', Name);
            std::regex exportClassRegex(R"(export\s+class\s+([a-zA-Z_$][a-zA-Z0-9_$]*))");
            std::sregex_iterator classIter(transformed.begin(), transformed.end(), exportClassRegex);
            std::sregex_iterator classEnd;

            std::vector<std::pair<size_t, std::pair<size_t, std::string>>> classReplacements;

            for (auto i = classIter; i != classEnd; ++i) {
                const std::smatch& match = *i;
                std::string className = match[1].str();
                size_t startPos = match.position();
                size_t matchLen = match.length();

                size_t searchStart = startPos + matchLen;
                size_t bracePos = transformed.find('{', searchStart);

                if (bracePos != std::string::npos) {
                    int braceCount = 1;
                    size_t endPos = bracePos + 1;

                    while (endPos < transformed.length() && braceCount > 0) {
                        if (transformed[endPos] == '{') braceCount++;
                        else if (transformed[endPos] == '}') braceCount--;
                        endPos++;
                    }

                    if (braceCount == 0) {
                        std::string replacement = "class " + className;
                        std::string exportCall = "\n__export('" + className + "', " + className + ");";

                        classReplacements.push_back({startPos, {matchLen, replacement}});
                        classReplacements.push_back({endPos, {0, exportCall}});
                    }
                }
            }

            std::sort(classReplacements.rbegin(), classReplacements.rend());
            for (const auto&[first, second] : classReplacements) {
                transformed.replace(first, second.first, second.second);
            }

            // export { x, y, z } -> __export('x', x); __export('y', y); __export('z', z);
            std::regex exportListRegex(R"(export\s*\{\s*([^}]+)\s*\}\s*;?)");
            std::sregex_iterator listIter(transformed.begin(), transformed.end(), exportListRegex);
            std::sregex_iterator listEnd;

            std::vector<std::pair<size_t, std::pair<size_t, std::string>>> listReplacements;
            for (auto i = listIter; i != listEnd; ++i) {
                const std::smatch& match = *i;
                std::string exportList = match[1].str();

                std::string exportCalls;
                std::regex identifierRegex(R"(([a-zA-Z_$][a-zA-Z0-9_$]*))");
                std::sregex_iterator identIter(exportList.begin(), exportList.end(), identifierRegex);
                std::sregex_iterator identEnd;

                for (auto j = identIter; j != identEnd; ++j) {
                    std::string identifier = (*j)[1].str();
                    if (!exportCalls.empty()) exportCalls += " ";
                    exportCalls += "__export('" + identifier + "', " + identifier + ");";
                }

                listReplacements.push_back({match.position(), {match.length(), exportCalls}});
            }

            std::sort(listReplacements.rbegin(), listReplacements.rend());
            for (const auto&[first, second] : listReplacements) {
                transformed.replace(first, second.first, second.second);
            }

            // export default x -> __exportDefault(x);
            std::regex exportDefaultRegex(R"(export\s+default\s+([^;]+);?)");
            transformed = std::regex_replace(transformed, exportDefaultRegex, "__exportDefault($1);");

            return transformed;
        }

        std::filesystem::path ResolveModulePath(const std::string& modulePath) const {
            if (modulePath.find("..") != std::string::npos ||
                modulePath.find("//") != std::string::npos ||
                modulePath.empty() ||
                modulePath[0] == '/' ||
                modulePath[0] == '\\') {
                m_Logger->Error("Invalid module path detected: %s", modulePath.c_str());
                return {};
            }

            std::filesystem::path resolved = m_ResourcePath / modulePath;

            std::error_code ec;
            resolved = std::filesystem::canonical(resolved, ec);
            if (ec) {
                resolved = m_ResourcePath / modulePath;
            } else {
                std::filesystem::path canonicalResourcePath = std::filesystem::canonical(m_ResourcePath, ec);
                if (ec || !IsSubPath(resolved, canonicalResourcePath)) {
                    m_Logger->Error("Module path escapes resource directory: %s", modulePath.c_str());
                    return {};
                }
            }

            std::vector<std::string> extensions = {"", ".js", ".mjs", ".json"};

            for (const auto& extension : extensions) {
                std::filesystem::path candidate = resolved;
                candidate += extension;

                if (std::filesystem::exists(candidate) && std::filesystem::is_regular_file(candidate)) {
                    return candidate;
                }
            }

            return {};
        }

        static bool IsSubPath(const std::filesystem::path& path, const std::filesystem::path& base) {
            const auto pathStr = path.string();
            auto baseStr = base.string();

            if (!baseStr.empty() && baseStr.back() != std::filesystem::path::preferred_separator) {
                baseStr += std::filesystem::path::preferred_separator;
            }

            return pathStr.size() >= baseStr.size() &&
                   pathStr.substr(0, baseStr.size()) == baseStr;
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

        void HandleException(const JSValueRef exception) const {
            if (!exception) {
                return;
            }

            const JSStringRef exceptionStr = JSValueToStringCopy(m_Context, exception, nullptr);
            if (exceptionStr) {
                const size_t bufferSize = JSStringGetMaximumUTF8CStringSize(exceptionStr);
                std::vector<char> buffer(bufferSize);
                JSStringGetUTF8CString(exceptionStr, buffer.data(), bufferSize);

                m_Logger->Error("JavaScript Error: %s", buffer.data());

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