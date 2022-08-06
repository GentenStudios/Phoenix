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
class MemoryHeap;
class Camera;
class Buffer;

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

	void CreateMemoryHeaps();

	void DestroyMemoryHeaps();

	void CreateCameraBuffer();

	void InitCamera();

	Window* mWindow;

	std::unique_ptr<RenderDevice> mDevice;
	std::unique_ptr<ResourceManager> mResourceManager;

	std::unique_ptr<MemoryHeap> mScratchGPUMemoryHeap;
	std::unique_ptr<MemoryHeap> mDeviceLocalMemoryHeap;
	std::unique_ptr<MemoryHeap> mGPUMappableMemoryHeap;

	RenderTarget* mPrimaryRenderTarget = nullptr;

	Camera* mCamera;
	Buffer* mCameraBuffer;

	static Phoenix* mInstance;
};

