workspace "Manna"
	architecture "x64"

	configurations { "Debug", "Release", "Dist" }
	startproject "Editor"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Manna_Engine"
	location "Manna_Engine"
	kind "SharedLib" 
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.cpp"
	}

	includedirs {
		"%{prj.name}/include", 
		"%{prj.name}/src"     
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines {
			"MN_PLATFORM_WINDOWS",
			"MN_DLL_EXPORT"
		}

	filter "system:linux"
		cppdialect "C++17"
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

project "Manna_Editor"
	location "Manna_Editor"
	kind "ConsoleApp" 
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files {
		"%{prj.name}/**.h",
		"%{prj.name}/**.cpp"
	}

	includedirs {
		"Manna_Engine/include",
	}

	links { "Manna_Engine" }

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines {
			"MN_PLATFORM_WINDOWS",
		}

		postbuildcommands {
			"{COPY} %{wks.location}bin/" .. outputdir .. "/Manna_engine/Manna_Engine.dll %{cfg.targetdir}"
		}

	filter "system:linux"
		cppdialect "C++17"
		staticruntime "On"

		defines {
			"MN_PLATFORM_LINUX",
		}

		links {
		}

		postbuildcommands {
			"{COPY} %{wks.location}bin/" .. outputdir .. "/Manna_engine/Manna_Engine.so %{cfg.targetdir}"
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