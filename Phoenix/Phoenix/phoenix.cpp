#include <phoenix.hpp>

#include <device.hpp>
#include <window.hpp>
#include <resourcemanager.hpp>
#include <rendertarget.hpp>
#include <renderpass.hpp>
#include <texture.hpp>
#include <globalresources.hpp>
#include <devicememory.hpp>
#include <camera.hpp>
#include <buffer.hpp>
#include <ResourceTable.hpp>
#include <rendertechnique.hpp>
#include <World.hpp>

Phoenix* Phoenix::mInstance = nullptr;

void WindowEvent(SDL_Event& event, void* ref)
{
	Phoenix* engine = reinterpret_cast<Phoenix*>(ref);

	switch (event.window.event)
	{
		//Get new dimensions and repaint on window size change
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		engine->Validate();
		break;
	}
}


Phoenix::Phoenix(Window* window) : mWindow(window)
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

	// Temporary global defition of all pipelines, will eventualy use the mod loader to load pipelines
	mResourceManager->LoadPipelineDictionary("Definitions.xml", GetPrimaryRenderTarget()->GetRenderPass());

	mInstance = this;
}

Phoenix::~Phoenix()
{
	mInstance = nullptr;

	mResourceManager.reset();

	DestroyMemoryHeaps();

	mWorld.reset();

	mDevice.reset();
}

void Phoenix::RebuildCommandBuffers()
{
	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = static_cast<float>(mWindow->GetWidth());
	viewport.height = static_cast<float>(mWindow->GetHeight());
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.extent.width = mWindow->GetWidth();
	scissor.extent.height = mWindow->GetHeight();
	scissor.offset.x = 0;
	scissor.offset.y = 0;

	// Make a basic command buffer
	VkCommandBuffer* commandBuffers = mDevice->GetPrimaryCommandBuffers();
	for (uint32_t i = 0; i < mDevice->GetSwapchainImageCount(); i++)
	{

		mDevice->Validate(vkResetCommandBuffer(
			commandBuffers[i],
			0
		));

		mDevice->BeginCommand(commandBuffers[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

		{
			mPrimaryRenderTarget->GetRenderPass()->Use(commandBuffers, i);

			vkCmdSetViewport(
				commandBuffers[i],
				0,
				1,
				&viewport
			);

			vkCmdSetScissor(
				commandBuffers[i],
				0,
				1,
				&scissor
			);

			// Example Rendering

			mWorld->Draw(commandBuffers, i);



			vkCmdEndRenderPass(
				commandBuffers[i]
			);
		}

		RenderTarget* src = mPrimaryRenderTarget;


		mDevice->TransitionImageLayout(commandBuffers[i], mDevice->GetSwapchainImages()[i], mDevice->GetSurfaceFormat(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
		src->GetImage()->TransitionImageLayout(commandBuffers[i], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkImageCopy copyRegion{};
		copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.srcOffset = { 0, 0, 0 };
		copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.dstOffset = { 0, 0, 0 };
		copyRegion.extent = { mDevice->GetWindowWidth(),  mDevice->GetWindowHeight(), 1 };
		vkCmdCopyImage(commandBuffers[i], src->GetImage()->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mDevice->GetSwapchainImages()[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		src->GetImage()->TransitionImageLayout(commandBuffers[i], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		mDevice->TransitionImageLayout(commandBuffers[i], mDevice->GetSwapchainImages()[i], mDevice->GetSurfaceFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });


		mDevice->Validate(vkEndCommandBuffer(
			commandBuffers[i]
		));
	}
}

void Phoenix::Update()
{
	UpdateCamera();
	mWorld->Update();
	mDevice->Present();
}

void Phoenix::Validate()
{
	mDevice->WindowChange(mWindow->GetWidth(), mWindow->GetHeight());

	RebuildRenderPassResources();
	RebuildCommandBuffers();
}

void Phoenix::UpdateCamera()
{
	mCamera->Update();
	mCameraBuffer->TransferInstantly(&mCamera->mCamera, sizeof(Camera::CameraPacket));
}

void Phoenix::RebuildRenderPassResources()
{
	mPrimaryRenderTarget->ScreenResize(
		mDevice->GetWindowWidth(),
		mDevice->GetWindowHeight()
	);
}

void Phoenix::CreateRenderPassResource()
{
	if (mPrimaryRenderTarget == nullptr)
	{
		mPrimaryRenderTarget = new RenderTarget(
			mDevice.get(),
			mDevice->GetWindowWidth(),
			mDevice->GetWindowHeight(),
			true
		);
		mResourceManager->RegisterResource<RenderTarget>(mPrimaryRenderTarget);
	}
	else
	{
		mPrimaryRenderTarget->ScreenResize(
			mDevice->GetWindowWidth(),
			mDevice->GetWindowHeight()
		);
	}
}

void Phoenix::CreateMemoryHeaps()
{
	uint32_t deviceLocalMemorySize = 40 * 1024 * 1024;
	uint32_t mappableMemorySize = 240 * 1024 * 1024;

	mDeviceLocalMemoryHeap = std::unique_ptr<MemoryHeap>(new MemoryHeap(mDevice.get(), deviceLocalMemorySize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
	mGPUMappableMemoryHeap = std::unique_ptr<MemoryHeap>(new MemoryHeap(mDevice.get(), mappableMemorySize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

	mResourceManager->RegisterResource<MemoryHeap>("DeviceLocalMemoryHeap", mDeviceLocalMemoryHeap.get(), false);
	mResourceManager->RegisterResource<MemoryHeap>("GPUMappableMemoryHeap", mGPUMappableMemoryHeap.get(), false);
}

void Phoenix::DestroyMemoryHeaps()
{
	mDeviceLocalMemoryHeap.reset();
	mGPUMappableMemoryHeap.reset();
}

void Phoenix::CreateCameraBuffer()
{
	mCameraBuffer = new Buffer(
		mDevice.get(), mGPUMappableMemoryHeap.get(), sizeof(Camera::CameraPacket),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE
	);
	mResourceManager->RegisterResource<Buffer>("CameraBuffer", mCameraBuffer);
	mResourceManager->GetResource<ResourceTable>("CameraResourceTable")->Bind(0, mCameraBuffer);
}

void Phoenix::InitCamera()
{
	mCamera = new Camera(mWindow->GetWidth(), mWindow->GetHeight());
	mCamera->Move(0.0f, 0.0f, 20.0f);
	mResourceManager->RegisterResource("Camera", mCamera, true);
}

void Phoenix::InitWorld()
{
	mWorld = std::unique_ptr<World>(new World(mDevice.get(), mGPUMappableMemoryHeap.get(), mResourceManager.get()));
	mResourceManager->RegisterResource("World", mWorld.get(), false);
}
