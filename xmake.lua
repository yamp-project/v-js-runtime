set_project("v-js-runtime")
set_languages("cxx23")

add_rules("mode.debug", "mode.release")

if is_os("windows") then
    add_toolchains("msvc")
elseif is_os("linux") then
    add_toolchains("clang")
end

option("static-client")
    set_description("Build static client library")
    set_default(false)
    set_showmenu(true)

target("yamp-sdk")
    set_kind("headeronly")
    add_headerfiles("vendors/yamp-sdk/**.h")
    on_config(function(target)
        local oldDir = os.cd("vendors/yamp-sdk")
        local out, err = os.iorun("git rev-parse --short HEAD")
        if err ~= "" then
            raise("Failed to get yamp-sdk git commit hash: " .. err)
            return
        end

        io.writefile("version.h", "#define YAMP_SDK_VERSION \"" .. string.trim(out) .. "\"")
        os.cd(oldDir)
    end)

target("shared")
    set_kind("headeronly")
    add_headerfiles("shared/src/**.h")
    add_deps("yamp-sdk")

    on_config(function(target)
        local out, err = os.iorun("git rev-parse --short HEAD")
        if err ~= "" then
            raise("Failed to get module git commit hash: " .. err)
            return
        end

        io.writefile("shared/src/version.h", "#define YAMP_RUNTIME_VERSION \"" .. string.trim(out) .. "\"")
    end)

target("server")
    set_basename("v-js-runtime-server")
    set_kind("shared")
    add_files("server/src/**.cpp", "shared/src/**.cpp")
    add_headerfiles("server/src/**.h")
    add_includedirs(
        "shared/src", "server/src",
        "vendors", "vendors/yamp-sdk"
    )

    add_deps("shared")
    add_defines("YAMP_SERVER")

    set_runtimes("MD")
    set_symbols("debug")

target("client")
    set_basename("v-js-runtime-client")
    if has_config("static-client") then
        set_kind("static")
    else
        set_kind("shared")
        add_defines("YAMP_SHARED")
    end

    add_files("client/src/**.cpp", "shared/src/**.cpp")
    add_headerfiles("client/src/**.h")
    add_includedirs(
        "shared/src", "client/src",
        "vendors", "vendors/yamp-sdk"
    )

    add_deps("shared")
    add_defines("YAMP_CLIENT")

    set_runtimes("MD")
    set_symbols("debug")
