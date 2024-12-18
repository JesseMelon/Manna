#pragma once

#ifdef MN_PLATFORM_WINDOWS
	#ifdef MN_BUILD_DLL
		#define MANNA_API __declspec(dllexport)
	#else
		#define MANNA_API __declspec(dllimport)
	#endif // MN_BUILD_DLL
#else
	#error Unsupported Platform
#endif // MN_PLATFORM_WINDOWS
