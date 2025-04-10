#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "core/memory.h"
#include "api_types.h"

//game is defined by client - vital to move main gameplay logic into engine code
extern b8 create_game(game* out_game);

/*
* --------------- Application entry point ---------------------
*/

int main() {

	init_memory(); //lowest level subsystem init to begin tracking memory usage

	game game_instance;

	//verify game is created
	if (!create_game(&game_instance)) {
		LOG_FATAL("Failed to create game.");
		return -1;
	}

	//verify function bindings
	if (!game_instance.initialize || !game_instance.update || !game_instance.render || !game_instance.on_resize) {
		LOG_FATAL("Invalid function pointers!");
		return -2;
	}

	//initialize application
	if (!create_application(&game_instance)) {
		LOG_INFO("Application failed to create.");
		return 1;
	}

	//begin game loop
	if (!run_application()) {
		LOG_INFO("Application did not shut down gracefully.");
		return 2;
	}

	shutdown_memory();
	return 0;
}
