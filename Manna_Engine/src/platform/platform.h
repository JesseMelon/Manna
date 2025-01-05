#pragma once
#ifdef MN_PLATFORM_WINDOWS
	#ifdef MN_DLL_EXPORT
		#define MANNA_API __declspec(dllexport)
	#else
		#define MANNA_API __declspec(dllimport)
	#endif
#else
	#define MANNA_API
#endif
