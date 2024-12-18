#include <Manna.h>

class Sandbox : public Manna::Application {
public:
	Sandbox() {

	}
	~Sandbox() {

	}
};

Manna::Application* Manna::CreateApplication() 
{
	return new Sandbox();
}