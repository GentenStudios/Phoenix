#pragma once

#include <Renderer/Vulkan.hpp>

class RenderDevice;
class ResourceManager;

void CreateGlobalResources( RenderDevice* device, ResourceManager* resourceManager );

