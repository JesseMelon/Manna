workspace "Manna"
	architecture "x64"

	configurations { "Debug", "Release", "Dist" }
	startproject "Editor"

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	-- Detect the platform and set the build system accordingly. this might all technically be unnessary since it detects ninja installation, and possibly also clang
    if _OPTIONS["os"] == "windows" then
        -- Visual Studio for Windows
        toolset "vstudio"
    elseif _OPTIONS["os"] == "linux" then
        -- Ninja for Linux
        makefiles "ninja"
        toolset "clang"
    elseif _OPTIONS["os"] == "macosx" then
        -- Ninja for macOS
        makefiles "ninja"
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
		"%{prj.name}/src"     
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines {
			"MN_PLATFORM_WINDOWS",
			"MN_DLL_EXPORT"
		}

	filter "system:linux"
		staticruntime "On"

		defines {
			"MN_PLATFORM_LINUX",
			"MN_SO_EXPORT"
		}

		links {
		}

	filter "configurations:Debug"
		defines "MN_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "MN_RELEASE"
		optimize "On"
	
	filter "configurations:Dist"
		defines "MN_DIST"
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
	}

	links { "manna_engine" }

	dependson { "manna_engine" }

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines {
			"MN_PLATFORM_WINDOWS",
		}

		postbuildcommands {
			"{COPY} %{wks.location}bin/" .. outputdir .. "/manna_engine/manna_engine.dll %{cfg.targetdir}"
		}

	filter "system:linux"
		staticruntime "On"

		defines {
			"MN_PLATFORM_LINUX",
		}

		postbuildcommands {
			"{COPY} %{wks.location}bin/" .. outputdir .. "/manna_engine/manna_engine.so %{cfg.targetdir}"
		}

	filter "configurations:Debug"
		defines "MN_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "MN_RELEASE"
		optimize "On"
	
	filter "configurations:Dist"
		defines "MN_DIST"
		optimize "On"