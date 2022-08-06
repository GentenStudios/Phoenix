#include <Phoenix/Phoenix.hpp>
#include <Windowing/Window.hpp>

#include <Renderer/Buffer.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/FramebufferAttachment.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/RenderTarget.hpp>
#include <Renderer/Renderpass.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/StaticMesh.hpp>
#include <Renderer/Texture.hpp>
#include <ResourceManager/RenderTechnique.hpp>

#include <lodepng.h>

#include <assert.h>
#include <memory>

std::unique_ptr<Window>       window;
std::unique_ptr<phx::Phoenix> engine;

int main(int, char**)
{
	const uint32_t width  = 1080;
	const uint32_t height = 720;

	window = std::unique_ptr<Window>(new Window("Phoenix", width, height));

	engine = std::unique_ptr<phx::Phoenix>(new phx::Phoenix(window.get()));

	engine->RebuildCommandBuffers();

	while (window->IsOpen())
	{
		window->Poll();

		if (window->IsRenderable())
		{
			engine->Update();
		}
	}

	engine.reset();

	window.reset();

	return 0;
}
