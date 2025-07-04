option("v8_build_mode")
    set_default("prebuilt")
    set_values("prebuilt", "source", "local")
    set_showmenu(true)
    set_category("v8")

option("v8_local_path")
    set_default("")
    set_showmenu(true)
    set_category("v8")

target("v8")
    set_kind("static")
    set_basename("v8_monolith")

    local install_dir = path.absolute("../../vendors/v8")
    add_includedirs(path.join(install_dir, "include"))
    add_linkdirs(path.join(install_dir, "lib"))
    add_links("v8_monolith")

    local v8_mode = get_config("v8_build_mode") or "prebuilt"
    
    if v8_mode == "prebuilt" then
        on_build(function (target)
            local function get_platform_info(target)
                local info = {}
                
                if target:is_plat("windows") then
                    info.platform = "windows"
                    info.arch = target:is_arch("x64") and "x64" or "x86"
                    info.lib_ext = ".lib"
                    info.dll_ext = ".dll"
                elseif target:is_plat("linux") then
                    info.platform = "linux"
                    info.arch = target:is_arch("x86_64") and "x64" or "x86"
                    info.lib_ext = ".a"
                    info.dll_ext = ".so"
                elseif target:is_plat("macosx") then
                    info.platform = "macos"
                    info.arch = target:is_arch("x86_64") and "x64" or "arm64"
                    info.lib_ext = ".a"
                    info.dll_ext = ".dylib"
                end
                
                return info
            end

            local function setup_depot_tools_env(depot_tools_path)
                local envs = {}
                local absolute_path = path.absolute(depot_tools_path)
                
                local current_path = os.getenv("PATH") or ""
                local separator = is_host("windows") and ";" or ":"
                envs.PATH = absolute_path .. separator .. current_path
                
                if is_host("windows") then
                    envs.DEPOT_TOOLS_WIN_TOOLCHAIN = "0"
                    envs.GYP_MSVS_VERSION = "2022"
                end
                
                envs.DEPOT_TOOLS_UPDATE = "0"
                envs.DEPOT_TOOLS_METRICS = "0"
                
                return envs
            end

            local function get_binary_url(platform_info)
                local base_url = "https://to-be-added.lol/v8/"
                local filename = string.format("v8-%s-%s", platform_info.platform, platform_info.arch)
                
                if platform_info.platform == "windows" then
                    return base_url .. filename .. ".zip"
                else
                    return base_url .. filename .. ".tar.gz"
                end
            end

            local function download_v8_headers(v8_dir, install_dir)
                import("net.http")
                import("utils.archive")

                v8_dir = path.absolute(v8_dir);

                local headers_url = "https://github.com/v8/v8/archive/refs/heads/main.zip"
                local headers_zip = path.join(v8_dir, "v8-headers.zip")
                
                if not os.exists(path.join(install_dir, "include", "v8.h")) then
                    print("Downloading V8 headers...")
                    http.download(headers_url, headers_zip)
                    
                    print(v8_dir)

                    os.cd(v8_dir)
                    archive.extract(headers_zip, ".")
                    
                    print(install_dir)

                    os.cp("v8-main/include", install_dir)

                    os.rm(headers_zip)
                    os.rmdir("v8-main")
                else
                    print("V8 headers already present, skipping download.")
                end
            end

            local function download_v8_binaries(v8_dir, install_dir, platform_info)
                import("utils.archive")
                local binary_url = get_binary_url(platform_info)
                local binary_file = path.join(v8_dir, "v8-binary" .. (platform_info.platform == "windows" and ".zip" or ".tar.gz"))
                
                if not os.exists(path.join(install_dir, "lib", "v8_monolith" .. platform_info.lib_ext)) then
                    print("Downloading V8 binaries for " .. platform_info.platform .. " " .. platform_info.arch .. "...")
                    
                    http.download(binary_url, binary_file)
                    
                    os.cd(v8_dir)
                    archive.extract(binary_file, ".")
                    
                    os.trycp("lib/*" .. platform_info.lib_ext, path.join(install_dir, "lib"))
                    if platform_info.platform == "windows" then
                        os.trycp("bin/*" .. platform_info.dll_ext, path.join(install_dir, "bin"))
                    end
                    
                    os.rm(binary_file)
                else
                    print("V8 binaries already present, skipping download.")
                end
            end

            print("=" .. string.rep("=", 50))
            print("Setting up V8 prebuilt binaries...")
            print("=" .. string.rep("=", 50))

            local v8_dir = "vendors/v8"
            local target_installdir = path.absolute(v8_dir)
            
            os.mkdir(path.join(target_installdir, "lib"))

            local platform_info = get_platform_info(target)
            
            download_v8_headers(v8_dir, target_installdir)
            
            download_v8_binaries(v8_dir, target_installdir, platform_info)
            
            print("Finished setting up V8 prebuilt binaries.")
        end)
    elseif v8_mode == "local" then
        on_build(function (target)
            local function get_platform_info(target)
                local info = {}
                
                if target:is_plat("windows") then
                    info.platform = "windows"
                    info.arch = target:is_arch("x64") and "x64" or "x86"
                    info.lib_ext = ".lib"
                    info.dll_ext = ".dll"
                elseif target:is_plat("linux") then
                    info.platform = "linux"
                    info.arch = target:is_arch("x86_64") and "x64" or "x86"
                    info.lib_ext = ".a"
                    info.dll_ext = ".so"
                elseif target:is_plat("macosx") then
                    info.platform = "macos"
                    info.arch = target:is_arch("x86_64") and "x64" or "arm64"
                    info.lib_ext = ".a"
                    info.dll_ext = ".dylib"
                end
                
                return info
            end

            local function setup_local_v8(v8_path, install_dir, target)
                print("Using local V8 installation at: " .. v8_path)
                
                os.mkdir(path.join(install_dir, "lib"))
                os.mkdir(path.join(install_dir, "bin"))
                
                local possible_include_paths = {
                    path.join(v8_path, "include"),
                    path.join(v8_path, "v8", "include"),
                    path.join(v8_path, "out.gn", "include"),
                    path.join(v8_path, "out", "Release", "include"),
                    path.join(v8_path, "out", "Debug", "include")
                }
                
                local possible_lib_paths = {
                    path.join(v8_path, "lib"),
                    path.join(v8_path, "out.gn", "obj"),
                    path.join(v8_path, "out", "Release", "obj"),
                    path.join(v8_path, "out", "Debug", "obj"),
                    path.join(v8_path, "build", "lib"),
                    v8_path
                }
                
                local header_found = false
                for _, include_path in ipairs(possible_include_paths) do
                    if os.exists(path.join(include_path, "v8.h")) then
                        print("Found V8 headers at: " .. include_path)
                        os.cp(include_path, install_dir)
                        header_found = true
                        break
                    end
                end
                
                if not header_found then
                    raise("Could not find V8 headers (v8.h) in local installation. Checked paths: " .. table.concat(possible_include_paths, ", "))
                end
                
                local platform_info = get_platform_info(target)
                local lib_pattern = "*v8_monolith*" .. platform_info.lib_ext
                local lib_found = false
                
                for _, lib_path in ipairs(possible_lib_paths) do
                    if os.exists(lib_path) then
                        local lib_files = os.files(path.join(lib_path, lib_pattern))
                        if #lib_files > 0 then
                            print("Found V8 libraries at: " .. lib_path)
                            os.cp(path.join(lib_path, lib_pattern), path.join(install_dir, "lib"))
                            lib_found = true
                            break
                        end
                    end 
                end 
                
                if not lib_found then
                    raise("Could not find V8 libraries (" .. lib_pattern .. ") in local installation. Checked paths: " .. table.concat(possible_lib_paths, ", "))
                end
            end

            print("=" .. string.rep("=", 50))
            print("Setting up V8 from local installation...")
            print("=" .. string.rep("=", 50))

            local v8_local_path = get_config("v8_local_path") or ""
            if v8_local_path == "" then
                raise("v8_local_path must be specified when using local V8 build mode")
            end

            if not os.exists(v8_local_path) then
                raise("Local V8 path does not exist: " .. v8_local_path)
            end

            setup_local_v8(v8_local_path, install_dir, target)
            
            print("Finished setting up V8 from local installation.")
        end)
    end

target("v8_from_source")
    set_kind("static")
    set_basename("v8_monolith")
    set_enabled(get_config("v8_build_mode") == "source")

    add_includedirs("include")
    add_links("v8_monolith")

    add_deps("depot_tools")

    on_build(function (target)
        print("=" .. string.rep("=", 50))
        print("Building V8 from source...")
        print("=" .. string.rep("=", 50))

        local v8_dir = "vendors/v8"
        local depot_tools_dir = "vendors/depot_tools"

        local envs = {}

        envs.DEPOT_TOOLS_PATH = depot_tools_dir

        if target:is_plat("windows") then
            envs.DEPOT_TOOLS_WIN_TOOLCHAIN = "0"
            envs.GYP_MSVS_VERSION = "2022"
        end

        local gclient = target:is_plat("windows") and "vendors/depot_tools/gclient.bat" or "vendors/depot_tools/gclient"

        os.setenv("PATH", path.join(path.absolute(path.directory(gclient))) .. ":", os.getenv("PATH"))

        local old_cwd = os.cd("vendors")

        if not (os.exists("v8") and os.isdir("v8")) then
            print("Downloading v8 source...")
            os.mkdir("v8")
            os.cd("v8")
            os.mkdir("source")
            os.cd("source")

            os.exec("fetch --no-history v8")
        else 
            os.cd("v8/source/v8")

            os.exec("git checkout main")
            os.exec("git fetch")
            os.exec("git pull")
            os.exec("gclient sync")
        end
        print("Done pulling V8 source")

        local configs = {
            treat_warnings_as_errors = false,
            is_official_build = false,
            is_component_build = false,
            is_debug = false,
            symbol_level = 0,
            strip_debug_info = true,
            treat_warnings_as_errors = false,
            use_custom_libcxx = false,
            v8_monolithic = true,
            v8_enable_sandbox = true,
            v8_enable_pointer_compression = true,
            v8_enable_webassembly = true,
            v8_enable_gdbjit = false,
            v8_enable_i18n_support = true,
            v8_enable_test_features = false,
            v8_use_external_startup_data = false
        }

        if target:is_plat("windows") then
            configs.is_clang = false
        elseif target:is_plat("linux") then
            configs.use_sysroot = false
            configs.is_clang = true
        end

        print("Running gn build...")
        local build_dir = "out.gn"

        local gn_args = {}
        for k, v in pairs(configs) do
            table.insert(gn_args, string.format("%s=%s", k, tostring(v)))
        end
        os.execv("gn", {"gen", build_dir, " --args=" .. table.concat(gn_args, " ")}, {envs = envs})

        os.runv("ninja", {"-C", build_dir, "v8_monolith"}, {envs = envs})

        os.cd(old_cwd)
        print("Finished building V8 from source.")
    end)