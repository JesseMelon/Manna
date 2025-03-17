#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "api_types.h"

//game is defined by client - vital to move main gameplay logic into engine code
extern b8 create_manna_app(manna_app* out_game);

/*
* --------------- Application entry point ---------------------
*/

int main() {

	manna_app game_instance;

	//verify game is created
	if (!create_manna_app(&game_instance)) {
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

	return 0;
}
