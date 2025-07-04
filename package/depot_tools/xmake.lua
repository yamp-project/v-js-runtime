target("depot_tools")
    set_kind("phony")
    on_build(function (target)
        local depot_tools_path = "vendors/depot_tools"
        print("Ensuring depot_tools directory exists: " .. depot_tools_path)
        os.mkdir(depot_tools_path)
        
        if not os.isdir(path.join(depot_tools_path, ".git")) then
            print("Cloning depot_tools repository...")
            os.run("git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git " .. depot_tools_path)
            print("depot_tools cloned successfully.")
        else
            print("depot_tools already cloned. Updating...")
            local old_cwd = os.cd(depot_tools_path)
            
            os.run("git reset --hard")
            os.run("git checkout main")
            os.run("git pull")
            
            if is_host("windows") then
                os.run("update_depot_tools.bat")
            else
                os.run("./update_depot_tools")
            end
            
            os.cd(old_cwd)
            print("depot_tools updated.")
        end
    end)