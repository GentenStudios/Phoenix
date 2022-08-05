
#pragma warning(disable : C46812)

#include <window.hpp>
#include <phoenix.hpp>

#include <device.hpp>
#include <renderpass.hpp>
#include <resourcetable.hpp>
#include <resourcetablelayout.hpp>
#include <pipeline.hpp>
#include <pipelinelayout.hpp>
#include <buffer.hpp>
#include <devicememory.hpp>
#include <staticmesh.hpp>
#include <texture.hpp>
#include <framebufferattachment.hpp>
#include <rendertarget.hpp>
#include <rendertechnique.hpp>

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