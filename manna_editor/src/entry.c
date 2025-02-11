#include "game.h"
#include <manna.h>

//define create_game 
b8 create_game(game* out_game) {

	out_game->app_config.title = "Manna Editor";
	out_game->app_config.x = 100;
	out_game->app_config.y = 100;
	out_game->app_config.width = 1280;
	out_game->app_config.height = 720;
	out_game->update = game_update;
	out_game->render = game_render;
	out_game->initialize = game_initialize;
	out_game->on_resize = game_on_resize;
	
	out_game->state = m_allocate(sizeof(game_state), MEMORY_TAG_UNKNOWN);
	
	return TRUE;
}