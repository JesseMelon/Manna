workspace "Manna"
    architecture "x64"
    cdialect "gnu17"
    configurations { "debug", "release", "dist" }
    startproject "Editor"
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    if _OPTIONS["os"] == "windows" then
        toolset "clang"
    elseif _OPTIONS["os"] == "linux" then
        toolset "clang"
    elseif _OPTIONS["os"] == "macosx" then
        toolset "clang"
    end

project "manna_engine"
    location "manna_engine"
    kind "SharedLib"
    language "C"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("obj/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/**.h",
        "%{prj.name}/**.c",
        "../assets/shaders/Builtin.ObjectShader.vert.glsl",
        "../assets/shaders/Builtin.ObjectShader.frag.glsl"
    }

    includedirs {
        "%{prj.name}/include",
        "%{prj.name}/src",
        os.getenv("VULKAN_SDK") .. "/include" or ""
    }

    buildoptions {"-Werror=vla"}

    local wksRoot = os.realpath(".")

    filter "system:windows"
        staticruntime "On"
        systemversion "latest"
        defines { "M_PLATFORM_WINDOWS", "M_DLL_EXPORT" }
        libdirs { os.getenv("VULKAN_SDK") .. "/Lib" or "" }
        links { "vulkan-1" }
        postbuildcommands {
            "mkdir \"" .. wksRoot .. "\\bin\\shaders\" 2>NUL & \"" .. os.getenv("VULKAN_SDK") .. "/bin/glslc.exe\" -fshader-stage=vert " .. wksRoot .. "/assets/shaders/Builtin.ObjectShader.vert.glsl -o " .. wksRoot .. "/bin/shaders/Builtin.ObjectShader.vert.spv",
            "\"" .. os.getenv("VULKAN_SDK") .. "/bin/glslc.exe\" -fshader-stage=frag " .. wksRoot .. "/assets/shaders/Builtin.ObjectShader.frag.glsl -o " .. wksRoot .. "/bin/shaders/Builtin.ObjectShader.frag.spv"
        }

    filter "system:linux"
        targetname = "libmanna_engine"
        staticruntime "On"
        defines { "M_PLATFORM_LINUX", "M_SO_EXPORT" }
        libdirs { os.getenv("VULKAN_SDK") .. "/lib" or "", "usr/X11R6/lib" }
        links { "vulkan", "xcb", "X11", "X11-xcb", "xkbcommon", "m" }
        linkoptions { ("-Wl,-rpath," .. os.getenv("VULKAN_SDK") .. "/lib") or "" }
        postbuildcommands {
            "mkdir -p " .. wksRoot .. "/bin/shaders && \"" .. os.getenv("VULKAN_SDK") .. "/bin/glslc\" -fshader-stage=vert " .. wksRoot .. "/assets/shaders/Builtin.ObjectShader.vert.glsl -o " .. wksRoot .. "/bin/shaders/Builtin.ObjectShader.vert.spv",
            "\"" .. os.getenv("VULKAN_SDK") .. "/bin/glslc\" -fshader-stage=frag " .. wksRoot .. "/assets/shaders/Builtin.ObjectShader.frag.glsl -o " .. wksRoot .. "/bin/shaders/Builtin.ObjectShader.frag.spv"
        }

--        postbuildcommands {
--            "\"" .. os.getenv("VULKAN_SDK") .. "/bin/glslc\" -fshader-stage=vert " .. wksRoot .. "/assets/shaders/Builtin.ObjectShader.vert.glsl -o " .. wksRoot .. "/bin/" .. outputdir .. "/manna_editor/Builtin.ObjectShader.vert.spv",
--            "\"" .. os.getenv("VULKAN_SDK") .. "/bin/glslc\" -fshader-stage=frag " .. wksRoot .. "/assets/shaders/Builtin.ObjectShader.frag.glsl -o " .. wksRoot .. "/bin/" .. outputdir .. "/manna_editor/Builtin.ObjectShader.frag.spv"
--        }

    filter "system:macosx"
        staticruntime "On"
        defines { "M_PLATFORM_MACOS", "M_SO_EXPORT" }
        libdirs { os.getenv("VULKAN_SDK") .. "/lib" or "" }
        links { "vulkan", "xcb", "X11", "X11-xcb", "xkbcommon", "m" }
        linkoptions { ("-Wl,-rpath," .. os.getenv("VULKAN_SDK") .. "/lib") or "" }
        postbuildcommands {
            "\"" .. os.getenv("VULKAN_SDK") .. "/bin/glslc\" -fshader-stage=vert " .. wksRoot .. "/assets/shaders/Builtin.ObjectShader.vert.glsl -o " .. wksRoot .. "/bin/" .. outputdir .. "/manna_editor/Builtin.ObjectShader.vert.spv",
            "\"" .. os.getenv("VULKAN_SDK") .. "/bin/glslc\" -fshader-stage=frag " .. wksRoot .. "/assets/shaders/Builtin.ObjectShader.frag.glsl -o " .. wksRoot .. "/bin/" .. outputdir .. "/manna_editor/Builtin.ObjectShader.frag.spv"
        }

    filter "configurations:debug"
        defines "DEBUG"
        symbols "On"

    filter "configurations:release"
        defines "RELEASE"
        optimize "On"

    filter "configurations:dist"
        defines "DIST"
        optimize "On"

    filter { "system:windows", "configurations:release"}
        buildoptions "/MT"

---------------------------------------------------------------------------------------------------------------------
--- Editor
---------------------------------------------------------------------------------------------------------------------
project "manna_editor"
    location "manna_editor"
    kind "ConsoleApp"
    language "C"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("obj/" .. outputdir .. "/%{prj.name}")
    debugdir ("bin/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/**.h",
        "%{prj.name}/**.c"
    }

    includedirs {
        "manna_engine/include",
        "manna_engine/src",
    }

    links { "manna_engine" }
    dependson { "manna_engine" }

    local wksRoot = os.realpath(".")

    filter "system:windows"
        staticruntime "On"
        systemversion "latest"
        defines { "M_PLATFORM_WINDOWS" }
        postbuildcommands {
            "{COPY} " .. wksRoot .. "/assets " .. wksRoot .. "/bin/" .. outputdir .. "/manna_editor /h /i /c /k /e /r /y",
            "{COPY} " .. wksRoot .. "/bin/" .. outputdir .. "/manna_engine/manna_engine.dll " .. wksRoot .. "/bin/" .. outputdir .. "/manna_editor"
        }

    filter "system:linux"
        targetname = "libmanna_editor"
        staticruntime "On"
        defines { "M_PLATFORM_LINUX" }
        postbuildcommands {
            --"cp -r " .. wksRoot .. "/assets/* " .. wksRoot .. "/bin/" .. outputdir .. "/manna_editor/ || echo 'Assets folder empty, nothing to copy'",
            "{COPY} " .. wksRoot .. "/bin/" .. outputdir .. "/manna_engine/libmanna_engine.so " .. wksRoot .. "/bin/" .. outputdir .. "/manna_editor"
        }

    filter "system:macosx"
        staticruntime "On"
        defines { "M_PLATFORM_MACOS" }
        postbuildcommands {
            --"cp -r " .. wksRoot .. "/assets/* " .. wksRoot .. "/bin/" .. outputdir .. "/manna_editor/ || echo 'Assets folder empty, nothing to copy'",
            "{COPY} " .. wksRoot .. "/bin/" .. outputdir .. "/manna_engine/manna_engine.dylib " .. wksRoot .. "/bin/" .. outputdir .. "/manna_editor"
        }

    filter "configurations:debug"
        defines "DEBUG"
        symbols "On"

    filter "configurations:release"
        defines "RELEASE"
        optimize "On"

    filter "configurations:dist"
        defines "DIST"
        optimize "On"

-------------------------------------------------------------------------------------------------------------------------------
--- Tests
-------------------------------------------------------------------------------------------------------------------------------
project "tests"
    location "tests"
    kind "ConsoleApp"
    language "C"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("obj/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.c"
    }

    includedirs {
        "%{prj.name}/src",
        "manna_engine/src"
    }

    links { "manna_engine" }
    dependson { "manna_engine" }

    buildoptions { "-g", "-Wno-missing-braces" }
    defines { "_DEBUG", "KIMPORT" }

    local wksRoot = os.realpath(".")

    filter "system:windows"
        staticruntime "On"
        systemversion "latest"
        defines { "M_PLATFORM_WINDOWS" }
        libdirs { wksRoot .. "/bin/" .. outputdir }
        postbuildcommands {
            "{COPY} " .. wksRoot .. "/bin/" .. outputdir .. "/manna_engine/manna_engine.dll " .. wksRoot .. "/bin/" .. outputdir .. "/tests"
        }

    filter "system:linux"
        targetname = "tests"
        staticruntime "On"
        defines { "M_PLATFORM_LINUX" }
        libdirs { wksRoot .. "/bin/" .. outputdir }
        postbuildcommands {
            "{COPY} " .. wksRoot .. "/bin/" .. outputdir .. "/manna_engine/libmanna_engine.so " .. wksRoot .. "/bin/" .. outputdir .. "/tests"
        }

    filter "system:macosx"
        staticruntime "On"
        defines { "M_PLATFORM_MACOS" }
        libdirs { wksRoot .. "/bin/" .. outputdir }
        postbuildcommands {
            "{COPY} " .. wksRoot .. "/bin/" .. outputdir .. "/manna_engine/manna_engine.dylib " .. wksRoot .. "/bin/" .. outputdir .. "/tests"
        }

    filter "configurations:debug"
        defines "DEBUG"
        symbols "On"

    filter "configurations:release"
        defines "RELEASE"
        optimize "On"

    filter "configurations:dist"
        defines "DIST"
        optimize "On"
