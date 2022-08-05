#pragma once

#include <vulkan.hpp>

#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <string>

#include <pugixml.hpp>

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>

#include <globals.hpp>

class Window;
class RenderDevice;
class ResourceManager;
class RenderTarget;

class Phoenix
{
public:

	Phoenix(Window* window);

	~Phoenix();

	void RebuildCommandBuffers();

	void Update();

	void Validate();

	RenderTarget* GetPrimaryRenderTarget() { return mPrimaryRenderTarget; }

private:

	void RebuildRenderPassResources();

	void CreateRenderPassResource();

	Window* mWindow;

	std::unique_ptr<RenderDevice> mDevice;
	std::unique_ptr<ResourceManager> mResourceManager;

	RenderTarget* mPrimaryRenderTarget = nullptr;

	static Phoenix* mInstance;
};

