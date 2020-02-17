#include <iostream>
#include <memory>

#include <Sprocket.h>

class ExampleLayer : public Sprocket::Layer
{
public:
	ExampleLayer() : Layer("Example") {};

	void onEvent(Sprocket::Event& event) override
	{
		SPKT_LOG_INFO("Got event {}", event.toString());
	}
};

class Workshop : public Sprocket::Application
{
};

int main(int argc, char* argv[])
{
    Workshop w;
    std::shared_ptr<Sprocket::Layer> layer(new ExampleLayer());
    w.pushLayer(layer);
    return Sprocket::begin(argc, argv, &w);
}