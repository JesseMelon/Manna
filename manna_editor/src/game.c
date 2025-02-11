#include "game.h"

b8 game_initialize(game* game_instance) {
	M_DEBUG("Game initialized");
	return TRUE;
}

b8 game_update(game* game_instance, f32 delta_time) {
	//M_DEBUG("Game updated");
	return TRUE;
}

b8 game_render(game* game_instance, f32 delta_time) {
	//M_DEBUG("Game rendered");
	return TRUE;
}

void game_on_resize(game* game_instance, u32 width, u32 height) {
	M_DEBUG("Game resized");
}
