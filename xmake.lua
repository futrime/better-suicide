add_rules("mode.debug", "mode.release", "mode.releasedbg")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_requires("levilamina 0.2.2")

target("better-suicide")
    add_cxflags(
        "/EHa"
    )
    add_files(
        "src/**.cpp"
    )
    add_includedirs(
        "src"
    )
    add_packages(
        "levilamina"
    )
    add_shflags(
        "/DELAYLOAD:bedrock_server.dll"
    )
    set_exceptions("none")
    set_kind("shared")
    set_languages("cxx23")

    after_build(function (target)
        local plugin_packer = import("scripts.after_build")

        local plugin_define = {
            pluginName = target:name(),
            pluginFile = path.filename(target:targetfile()),
        }
        
        plugin_packer.pack_plugin(target,plugin_define)
    end)
