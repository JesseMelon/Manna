#include <manna.h>

//resume ep 6

int main() {
	
	application_create_info config;
	config.title = "Manna Editor";
	config.x = 100;
	config.y = 100;
	config.width = 1280;
	config.height = 720;

	M_DEBUG("%d", create_application(&config));

	run_application();

	return 0;
}