#include <Renderer/PostFX.hpp>

#include <Renderer/Device.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/RenderTarget.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/StaticMesh.hpp>

PostFX::PostFX(RenderDevice* device) : mDevice(device) {}
