#pragma once

#include <Renderer/Vulkan.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>

#include <Globals/Globals.hpp>

class Window;
class RenderDevice;
class ResourceManager;
class RenderTarget;
class MemoryHeap;
class Camera;
class RenderTechnique;
class Buffer;

namespace phx
{
	class World;

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
		void UpdateCamera();

		void RebuildRenderPassResources();

		void CreateRenderPassResource();

		void CreateMemoryHeaps();

		void DestroyMemoryHeaps();

		void CreateCameraBuffer();

		void InitCamera();

		void InitWorld();

		Window* mWindow;

		std::unique_ptr<RenderDevice>    mDevice;
		std::unique_ptr<ResourceManager> mResourceManager;

		std::unique_ptr<World> mWorld;

		std::unique_ptr<MemoryHeap> mScratchGPUMemoryHeap;
		std::unique_ptr<MemoryHeap> mDeviceLocalMemoryHeap;
		std::unique_ptr<MemoryHeap> mGPUMappableMemoryHeap;

		RenderTarget* mPrimaryRenderTarget = nullptr;

		Camera* mCamera;
		Buffer* mCameraBuffer;

		static Phoenix* mInstance;
	};
} // namespace phx
