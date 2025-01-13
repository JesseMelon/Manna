#include <manna.h>

int main() {
	//test();

	M_FATAL("Fatal! %f:", 1.0);
	M_ERROR("Error!");
	M_WARN("Warn!");
	M_DEBUG("Debug!");
	M_INFO("Info!");
	M_TRACE("Trace!");

	M_ASSERT(1 == 0, "Assert!");

	return 0;
}