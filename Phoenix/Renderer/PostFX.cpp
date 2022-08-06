#include <Renderer/PostFX.hpp>

#include <Renderer/Device.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/StaticMesh.hpp>
#include <Renderer/RenderTarget.hpp>

PostFX::PostFX( RenderDevice* device ) : mDevice( device )
{
}
