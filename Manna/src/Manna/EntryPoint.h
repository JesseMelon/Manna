#pragma once

#ifdef MN_PLATFORM_WINDOWS

extern Manna::Application* Manna::CreateApplication();

int main(int argc, char** argv) 
{
	auto app = Manna::CreateApplication();
	app->Run();
	delete app;
}

#endif