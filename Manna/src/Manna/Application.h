#pragma once

#include "Core.h"

namespace Manna {
	class MANNA_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	//to be defined in client
	Application* CreateApplication();

}

