#include "app.h"
#include <manna.h>

//define create_game 
b8 create_manna_app(manna_app* out_app) {

	out_app->app_config.title = "Manna Editor";
	out_app->app_config.x = 100;
	out_app->app_config.y = 100;
	out_app->app_config.width = 1280;
	out_app->app_config.height = 720;
	out_app->update = update_app;
	out_app->render = render_app;
	out_app->initialize = initialize_app;
	out_app->on_resize = app_on_resize;
	
	out_app->state = m_allocate(sizeof(state), MEMORY_TAG_EDITOR);
    out_app->application_state = 0;
	
	return TRUE;
}
