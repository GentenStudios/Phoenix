#include <Phoenix/Phoenix.hpp>

#include <Phoenix/World.hpp>
#include <Phoenix/DebugUI.hpp>
#include <Phoenix/DebugWindows.hpp>

#include <Renderer/Buffer.hpp>
#include <Renderer/Camera.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/RenderTarget.hpp>
#include <Renderer/Renderpass.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/Texture.hpp>

#include <ResourceManager/GlobalResources.hpp>
#include <ResourceManager/RenderTechnique.hpp>

#include <ResourceManager/ResourceManager.hpp>
#include <Windowing/Window.hpp>

#include <lodepng.h>

phx::Phoenix* phx::Phoenix::mInstance = nullptr;

void WindowEvent(SDL_Event& event, void* ref)
{
	phx::Phoenix* engine = reinterpret_cast<phx::Phoenix*>(ref);

	switch (event.window.event)
	{
		// Get new dimensions and repaint on window size change
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		engine->Validate();
		break;
	}
}


phx::Phoenix::Phoenix(Window* window) : mWindow(window)
{
	window->AddEventCallback(SDL_WINDOWEVENT, WindowEvent, this);

	mDevice = std::unique_ptr<RenderDevice>(new RenderDevice(window, window->GetWidth(), window->GetHeight()));

	mResourceManager = std::unique_ptr<ResourceManager>(new ResourceManager(mDevice.get()));

	CreateRenderPassResource();
	CreateGlobalResources(mDevice.get(), mResourceManager.get());
	CreateMemoryHeaps();

	CreateCameraBuffer();
	InitCamera();
	InitWorld();
	InitDebugUI();
	InitTexturePool();
	InitDefaultTextures();

	// Temporary global defition of all pipelines, will eventualy use the mod loader to load pipelines
	mResourceManager->LoadPipelineDictionary("Definitions.xml", GetPrimaryRenderTarget()->GetRenderPass());

	mInstance = this;
}

phx::Phoenix::~Phoenix()
{
	mInstance = nullptr;

	mResourceManager.reset();

	DestroyMemoryHeaps();

	mWorld.reset();

	mDebugUI.reset();

	mDevice.reset();
}

void phx::Phoenix::RebuildCommandBuffers()
{
	VkViewport viewport = {};
	viewport.x          = 0;
	viewport.y          = 0;
	viewport.width      = static_cast<float>(mWindow->GetWidth());
	viewport.height     = static_cast<float>(mWindow->GetHeight());
	viewport.minDepth   = 0.0f;
	viewport.maxDepth   = 1.0f;

	VkRect2D scissor {};
	scissor.extent.width  = mWindow->GetWidth();
	scissor.extent.height = mWindow->GetHeight();
	scissor.offset.x      = 0;
	scissor.offset.y      = 0;

	// Make a basic command buffer
	VkCommandBuffer* commandBuffers = mDevice->GetPrimaryCommandBuffers();
	for (uint32_t i = 0; i < mDevice->GetSwapchainImageCount(); i++)
	{

		mDevice->Validate(vkResetCommandBuffer(commandBuffers[i], 0));

		mDevice->BeginCommand(commandBuffers[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		{
			mPrimaryRenderTarget->GetRenderPass()->Use(commandBuffers, i);

			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			// Example Rendering

			mWorld->Draw(commandBuffers, i);

			mDebugUI->Use(commandBuffers, i, true);

			vkCmdEndRenderPass(
				commandBuffers[i]
			);
		}

		RenderTarget* src = mPrimaryRenderTarget;

		mDevice->TransitionImageLayout(commandBuffers[i], mDevice->GetSwapchainImages()[i], mDevice->GetSurfaceFormat(),
		                               VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		                               {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
		src->GetImage()->TransitionImageLayout(commandBuffers[i], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkImageCopy copyRegion {};
		copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
		copyRegion.srcOffset      = {0, 0, 0};
		copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
		copyRegion.dstOffset      = {0, 0, 0};
		copyRegion.extent         = {mDevice->GetWindowWidth(), mDevice->GetWindowHeight(), 1};
		vkCmdCopyImage(commandBuffers[i], src->GetImage()->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		               mDevice->GetSwapchainImages()[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		src->GetImage()->TransitionImageLayout(commandBuffers[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		                                       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		mDevice->TransitionImageLayout(commandBuffers[i], mDevice->GetSwapchainImages()[i], mDevice->GetSurfaceFormat(),
		                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		                               {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

		mDevice->Validate(vkEndCommandBuffer(commandBuffers[i]));
	}
}

void phx::Phoenix::Update()
{
	UpdateCamera();
	mWorld->Update();

	mStatisticManager.StartStatistic("ImGui");
	// Temp delta time
	mDebugUI->Update(0.01f);
	mStatisticManager.StopStatistic("ImGui");

	if (mDebugUI->IsCMDOutdated())
	{
		RebuildCommandBuffers();
	}

	mStatisticManager.StartStatistic("Render");
	mDevice->Present();
	mStatisticManager.StopStatistic("Render");
	mStatisticManager.StopStatistic("Total Frametime");

	mStatisticManager.StartStatistic("Total Frametime");
	mStatisticManager.Update();
}

void phx::Phoenix::Validate()
{
	mCamera->SetProjection(mWindow->GetWidth(), mWindow->GetHeight());
	mDevice->WindowChange(mWindow->GetWidth(), mWindow->GetHeight());
	mDebugUI->ViewportResize();
	RebuildRenderPassResources();
	RebuildCommandBuffers();
}

void phx::Phoenix::UpdateCamera()
{
	mCamera->Update();
	mCameraBuffer->TransferInstantly(&mCamera->mCamera, sizeof(Camera::CameraPacket));
}

void phx::Phoenix::RebuildRenderPassResources()
{
	mPrimaryRenderTarget->ScreenResize(mDevice->GetWindowWidth(), mDevice->GetWindowHeight());
}

void phx::Phoenix::CreateRenderPassResource()
{
	if (mPrimaryRenderTarget == nullptr)
	{
		mPrimaryRenderTarget = new RenderTarget(mDevice.get(), mDevice->GetWindowWidth(), mDevice->GetWindowHeight(), true);
		mResourceManager->RegisterResource<RenderTarget>(mPrimaryRenderTarget);
	}
	else
	{
		mPrimaryRenderTarget->ScreenResize(mDevice->GetWindowWidth(), mDevice->GetWindowHeight());
	}
}

void phx::Phoenix::CreateMemoryHeaps()
{
	uint32_t deviceLocalMemorySize = 40 * 1024 * 1024;
	uint32_t mappableMemorySize    = 200 * 1024 * 1024;

	mDeviceLocalMemoryHeap =
	    std::unique_ptr<MemoryHeap>(new MemoryHeap(mDevice.get(), deviceLocalMemorySize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
	mGPUMappableMemoryHeap = std::unique_ptr<MemoryHeap>(
	    new MemoryHeap(mDevice.get(), mappableMemorySize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

	mResourceManager->RegisterResource<MemoryHeap>("DeviceLocalMemoryHeap", mDeviceLocalMemoryHeap.get(), false);
	mResourceManager->RegisterResource<MemoryHeap>("GPUMappableMemoryHeap", mGPUMappableMemoryHeap.get(), false);
}

void phx::Phoenix::DestroyMemoryHeaps()
{
	mDeviceLocalMemoryHeap.reset();
	mGPUMappableMemoryHeap.reset();
}

void phx::Phoenix::CreateCameraBuffer()
{
	mCameraBuffer = new Buffer(mDevice.get(), mGPUMappableMemoryHeap.get(), sizeof(Camera::CameraPacket),
	                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
	mResourceManager->RegisterResource<Buffer>("CameraBuffer", mCameraBuffer);
	mResourceManager->GetResource<ResourceTable>("CameraResourceTable")->Bind(0, mCameraBuffer);
}

void phx::Phoenix::InitCamera()
{
	mCamera = new Camera(mWindow->GetWidth(), mWindow->GetHeight());
	mCamera->Move(0.0f, 0.0f, 20.0f);
	mResourceManager->RegisterResource("Camera", mCamera, true);
}

void phx::Phoenix::InitWorld()
{
	mWorld = std::unique_ptr<World>(new World(mDevice.get(), mGPUMappableMemoryHeap.get(), mResourceManager.get()));
	mResourceManager->RegisterResource("World", mWorld.get(), false);
}

void phx::Phoenix::InitDebugUI()
{
	mDebugUI = std::unique_ptr<DebugUI>(new DebugUI(mDevice.get(), mResourceManager.get(), mWindow));

	mResourceManager->RegisterResource("DebugUI", mDebugUI.get(), false);

	mDebugUI->AddMainmenuCallback(DebugUIMainMenuBar, this);

	
	mDebugUI->AddRenderCallback(DebugUIRenderSystemStatistics, this);

	mDebugUI->AddRenderCallback(DebugUIMemoryUsage, this);
	
}

void phx::Phoenix::InitTexturePool() 
{
	ResourceTableLayout* samplerArrayResourceTableLayout = mResourceManager->GetResource<ResourceTableLayout>("SamplerArrayResourceTableLayout");

	ResourceTable* defaultSamplerArrayResourceTable = samplerArrayResourceTableLayout->CreateTable();
	mResourceManager->RegisterResource<ResourceTable>("SamplerArrayResourceTable", defaultSamplerArrayResourceTable);
}

void phx::Phoenix::InitDefaultTextures() 
{
	ResourceTable* defaultSamplerArrayResourceTable = mResourceManager->GetResource<ResourceTable>("SamplerArrayResourceTable");

	const char errorTextureData[] = {
		0xFF,0x00,0xFF,0xFF
	};
	
	Texture* errorTexture = new Texture(
		mDevice.get(), 
		mDeviceLocalMemoryHeap.get(),
		1,
		1,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_SAMPLED_BIT,
		(char*)(errorTextureData));

	mResourceManager->RegisterResource<Texture>("ErrorTexture", errorTexture);

	for (int i = 0; i < MAX_SPRITESHEET_SAMPLER_ARRAY; ++i)
	{
		defaultSamplerArrayResourceTable->Bind(0, errorTexture, i);
	}


	// Load temporary textures
	const char* paths[2] = {"data/Textures/dirt.png", "data/Textures/stone.png"};
	for (int i = 0 ; i < 2; i++)
	{
		std::vector<unsigned char> image_data;
		uint32_t                   width, height;
		unsigned                   error = lodepng::decode(image_data, width, height, paths[i]);
		if (error)
		{
			printf("%s\n", lodepng_error_text(error));
			
		}
		else
		{
			Texture* blockTexture = new Texture(
				mDevice.get(),
				mDeviceLocalMemoryHeap.get(),
				width,
				height,
				VK_FORMAT_R8G8B8A8_UNORM,
			    VK_IMAGE_USAGE_SAMPLED_BIT,
				(char*) (image_data.data()));

			// For memory cleanup
			mResourceManager->RegisterResource<Texture>(blockTexture);

			defaultSamplerArrayResourceTable->Bind(0, blockTexture, i);
		}





	}

}
