#pragma once

#include <vulkan.hpp>

class RenderDevice;
class ResourceManager;

void CreateGlobalResources( RenderDevice* device, ResourceManager* resourceManager );

