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
		"%{prj.name}/**.c"
	}

	includedirs {
		"%{prj.name}/include",
		"%{prj.name}/src",
        os.getenv("VULKAN_SDK") .. "/include" or "" --if Vulkan SDK is not installed, the <or ""> prevents an error, but it will need to be installed to compile
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines {
			"M_PLATFORM_WINDOWS",
			"M_DLL_EXPORT"
		}
        libdirs {
            os.getenv("VULKAN_SDK") .. "/Lib" or ""
        }
        links {
            "vulkan-1"
        }

	filter "system:linux"
        targetname = "libmanna_engine"
		staticruntime "On"

		defines {
			"M_PLATFORM_LINUX",
			"M_SO_EXPORT"
		}
        libdirs {
            os.getenv("VULKAN_SDK") .. "/lib" or "",
            "usr/X11R6/lib"
        }
		links {
            "vulkan",
            "xcb",
            "X11",
            "X11-xcb",
            "xkbcommon",
            "m"
		}
        linkoptions {
            ("-Wl,-rpath," .. os.getenv("VULKAN_SDK") .. "/lib") or ""
        }
    filter "system:macosx"
        staticruntime "On"
        defines {
            "M_PLATFORM_MACOS",
            "M_SO_EXPORT"
        }
        libdirs {
            os.getenv("VULKAN_SDK") .. "/lib" or ""
        }
        links {
            "vulkan", ---lvulkan
            "xcb",
            "X11",
            "X11-xcb",
            "xkbcommon",
            "m"
        }
        linkoptions {
            ("-Wl,-rpath," .. os.getenv("VULKAN_SDK") .. "/lib") or ""
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

	filter { "system:windows", "configurations:Release"}
		buildoptions "/MT"

project "manna_editor"
	location "manna_editor"
	kind "ConsoleApp"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.c"
	}

	includedirs {
		"manna_engine/include",
        "manna_engine/src", --TODO temporary, this is just exposing internal engine stuff until the API is carved out more
	}

	links { "manna_engine" }

	dependson { "manna_engine" }

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines {
			"M_PLATFORM_WINDOWS",
		}

		postbuildcommands {
			"{COPY} %{wks.location}/bin/" .. outputdir .. "/manna_engine/manna_engine.dll %{cfg.targetdir}"
		}

	filter "system:linux"
        targetname = "libmanna_editor"
		staticruntime "On"

		defines {
			"M_PLATFORM_LINUX",
		}

		postbuildcommands {
			"{COPY} %{wks.location}/bin/" .. outputdir .. "/manna_engine/libmanna_engine.so %{cfg.targetdir}"
		}

    filter "system:macosx"
        staticruntime "On"

        defines {
            "M_PLATFORM_MACOS"
        }

        postbuildcommands {
            "{COPY} %{wks.location}/bin/" .. outputdir .. "/manna_engine/manna_engine.dylib %{cfg.targetdir}"
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
