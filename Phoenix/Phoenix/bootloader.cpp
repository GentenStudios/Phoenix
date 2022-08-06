#include <Windowing/window.hpp>
#include <Phoenix/phoenix.hpp>

#include <Renderer/device.hpp>
#include <Renderer/renderpass.hpp>
#include <Renderer/resourcetable.hpp>
#include <Renderer/resourcetablelayout.hpp>
#include <Renderer/pipeline.hpp>
#include <Renderer/pipelinelayout.hpp>
#include <Renderer/buffer.hpp>
#include <Renderer/devicememory.hpp>
#include <Renderer/staticmesh.hpp>
#include <Renderer/texture.hpp>
#include <Renderer/framebufferattachment.hpp>
#include <Renderer/rendertarget.hpp>
#include <ResourceManager/rendertechnique.hpp>

#include <lodepng.h>

#include <memory>
#include <assert.h>

std::unique_ptr<Window> window;
std::unique_ptr<Phoenix> engine;

int main( int, char** )
{

	const uint32_t width = 1080;
	const uint32_t height = 720;

	window = std::unique_ptr<Window>( new Window( "Phoenix", width, height ) );

	engine = std::unique_ptr<Phoenix>( new Phoenix( window.get() ) );
	
	engine->RebuildCommandBuffers( );

	while ( window->IsOpen() )
	{
		window->Poll( );

		if (window->IsRenderable())
		{
			engine->Update();
		}

	}

	engine.reset( );

	window.reset( );

	return 0;
}