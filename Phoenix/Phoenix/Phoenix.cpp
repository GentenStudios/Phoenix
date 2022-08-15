#include <Phoenix/Phoenix.hpp>

#include <Phoenix/DebugUI.hpp>
#include <Phoenix/DebugWindows.hpp>
#include <Phoenix/InputHandler.hpp>
#include <Phoenix/Mods.hpp>
#include <Phoenix/World.hpp>

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

void RenderSystemStatistics(void* ref)
{
	phx::Phoenix* engine = reinterpret_cast<phx::Phoenix*>(ref);

	ImGui::SetNextWindowPos(ImVec2(20, 20));

	static bool      open = true;
	ImGuiWindowFlags flags =
	    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	if (!ImGui::Begin("Render Statistics", &open, flags))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("%s: %.5g", "FPS", 420.69f);

	ImGui::SetWindowSize(ImVec2(220, ImGui::GetCursorPosY()));

	ImGui::End();
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
	InitMods();
	InitWorld();
	InitDebugUI();
	InitInputHandler();
	InitTexturePool();
	InitDefaultTextures();

	// Temporary global defition of all pipelines, will eventualy use the mod loader to load pipelines
	mResourceManager->LoadPipelineDictionary("Definitions.xml", GetPrimaryRenderTarget()->GetRenderPass());

	mDeltaTime = 0.0f;
	mInstance  = this;
}

phx::Phoenix::~Phoenix()
{
	mInstance = nullptr;

	mResourceManager.reset();

	DestroyMemoryHeaps();

	mWorld.reset();

	mDebugUI.reset();

	mInputHandler.reset();

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

		mWorld->ComputeVisibility(commandBuffers, i);

		{
			mPrimaryRenderTarget->GetRenderPass()->Use(commandBuffers, i);

			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			mWorld->Draw(commandBuffers, i);

			mDebugUI->Use(commandBuffers, i, true);

			vkCmdEndRenderPass(commandBuffers[i]);
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
	mDebugUI->Update(mDeltaTime);
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

	mDeltaTime = GetStatistics().GetStatisticSecconds("Total Frametime");

	// Must come after all mouse move reads
	mInputHandler->Update();
}

void phx::Phoenix::Validate()
{
	mCamera->SetProjection(mWindow->GetWidth(), mWindow->GetHeight());
	mDevice->WindowChange(mWindow->GetWidth(), mWindow->GetHeight());
	mDebugUI->ViewportResize();
	RebuildRenderPassResources();
	RebuildCommandBuffers();
}

Window* phx::Phoenix::GetWindow() { return mWindow; }

void phx::Phoenix::UpdateCamera()
{
	float movmentSpeed = 5.0f;

	if (mInputHandler->IsPressed(SDL_SCANCODE_LALT))
	{
		mWindow->GrabMouse(false);
	}
	else
	{
		mWindow->GrabMouse(true);

		if (mInputHandler->IsPressed(SDL_SCANCODE_LSHIFT))
		{
			movmentSpeed *= 3;
		}
		if (mInputHandler->IsPressed(SDL_SCANCODE_W))
		{
			mCamera->MoveLocalZ(movmentSpeed * mDeltaTime);
		}

		if (mInputHandler->IsPressed(SDL_SCANCODE_S))
		{
			mCamera->MoveLocalZ(-movmentSpeed * mDeltaTime);
		}

		if (mInputHandler->IsPressed(SDL_SCANCODE_A))
		{
			mCamera->MoveLocalX(movmentSpeed * mDeltaTime);
		}

		if (mInputHandler->IsPressed(SDL_SCANCODE_D))
		{
			mCamera->MoveLocalX(-movmentSpeed * mDeltaTime);
		}

		if (mInputHandler->IsPressed(SDL_SCANCODE_LCTRL))
		{
			mCamera->MoveWorldY(-movmentSpeed * mDeltaTime);
		}

		if (mInputHandler->IsPressed(SDL_SCANCODE_SPACE))
		{
			mCamera->MoveWorldY(movmentSpeed * mDeltaTime);
		}

		float mouseMoveX = mInputHandler->GetMouseMoveX();
		if (mouseMoveX != 0.0f)
		{
			mCamera->RotateYaw(mouseMoveX);
		}

		float mouseMoveY = mInputHandler->GetMouseMoveY();
		if (mouseMoveY != 0.0f)
		{
			mCamera->RotatePitch(mouseMoveY);
		}

		{
			static bool wasPressed = false;
			if (mInputHandler->IsMousePressed(0))
			{
				if (!wasPressed)
				{
					mWorld->DestroyBlockFromView();
				}
				wasPressed = true;
			}
			else
			{
				wasPressed = false;
			}
		}

		{
			static bool wasPressed = false;
			if (mInputHandler->IsMousePressed(1))
			{
				if (!wasPressed)
				{
					mWorld->PlaceBlockFromView();
				}
				wasPressed = true;
			}
			else
			{
				wasPressed = false;
			}
		}
	}

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
	mCamera->SetWorldPosition(glm::vec3(0.0f, 5.5f, 20.0f));
	mResourceManager->RegisterResource("Camera", mCamera, true);
}

void phx::Phoenix::InitWorld()
{
	mWorld = std::unique_ptr<World>(new World(mDevice.get(), mGPUMappableMemoryHeap.get(), mResourceManager.get()));
	mResourceManager->RegisterResource("World", mWorld.get(), false);
}

void phx::Phoenix::InitMods()
{
	int modCount = 1;

	mMods = std::unique_ptr<ModHandler>(new ModHandler(modCount));
	mResourceManager->RegisterResource("ModHandler", mMods.get(), false);

	mMods->AddMod("mods/standard_blocks-0.1/standard_blocks.xml");
}

void phx::Phoenix::InitDebugUI()
{
	mDebugUI = std::unique_ptr<DebugUI>(new DebugUI(mDevice.get(), mResourceManager.get(), mWindow));

	mResourceManager->RegisterResource("DebugUI", mDebugUI.get(), false);

	mDebugUI->AddMainmenuCallback(DebugUIMainMenuBar, this);

	mDebugUI->AddRenderCallback(DebugUIRenderSystemStatistics, this);

	mDebugUI->AddRenderCallback(DebugUIMemoryUsage, this);
}

void phx::Phoenix::InitInputHandler()
{
	mInputHandler = std::unique_ptr<InputHandler>(new InputHandler(mWindow));

	mResourceManager->RegisterResource<InputHandler>("InputHandler", mInputHandler.get(), false);
}

void phx::Phoenix::InitTexturePool()
{
	ResourceTableLayout* samplerArrayResourceTableLayout =
	    mResourceManager->GetResource<ResourceTableLayout>("SamplerArrayResourceTableLayout");

	ResourceTable* defaultSamplerArrayResourceTable = samplerArrayResourceTableLayout->CreateTable();
	mResourceManager->RegisterResource<ResourceTable>("SamplerArrayResourceTable", defaultSamplerArrayResourceTable);
}

void phx::Phoenix::InitDefaultTextures()
{
	ResourceTable* defaultSamplerArrayResourceTable = mResourceManager->GetResource<ResourceTable>("SamplerArrayResourceTable");

	const char errorTextureData[] = {0xFF, 0x00, 0xFF, 0xFF};

	Texture* errorTexture = new Texture(mDevice.get(), mDeviceLocalMemoryHeap.get(), 1, 1, VK_FORMAT_R8G8B8A8_UNORM,
	                                    VK_IMAGE_USAGE_SAMPLED_BIT, (char*) (errorTextureData));

	mResourceManager->RegisterResource<Texture>("ErrorTexture", errorTexture);

	for (int i = 0; i < MAX_SPRITESHEET_SAMPLER_ARRAY; ++i)
	{
		defaultSamplerArrayResourceTable->Bind(0, errorTexture, i);
	}

	// Temporarily load singular textures from ModHandler.
	const int  modCount = mMods->GetModCount();
	const Mod* mods     = mMods->GetMods();

	// Start at 1 bc 0 is error.
	unsigned int textureCount = 1;
	for (int i = 0; i < modCount; ++i)
	{
		const unsigned int blockCount = mods[i].blocks.GetBlockCount();
		Block*       blocks     = mods[i].blocks.GetBlocks();

		for (int j = 0; j < blockCount; ++j)
		{
			if (blocks[j].texture.empty())
				continue;

			std::vector<unsigned char> imageData;
			uint32_t                   width, height;
			const unsigned int         error = lodepng::decode(imageData, width, height, blocks[j].texture);

			if (error)
			{
				printf("%s\n", lodepng_error_text(error));
			}
			else
			{
				Texture* blockTexture = new Texture(mDevice.get(), mDeviceLocalMemoryHeap.get(), width, height, VK_FORMAT_R8G8B8A8_UNORM,
				                                    VK_IMAGE_USAGE_SAMPLED_BIT, reinterpret_cast<char*>(imageData.data()));

				// For memory cleanup
				mResourceManager->RegisterResource<Texture>(blockTexture);

				blocks[j].textureIndex = textureCount;
				defaultSamplerArrayResourceTable->Bind(0, blockTexture, textureCount++);
			}
		}
	}

	// Load skybox textures.
	const char* skybox[] = {"mods/standard_blocks-0.1/textures/north.png", "mods/standard_blocks-0.1/textures/east.png",
	                        "mods/standard_blocks-0.1/textures/south.png", "mods/standard_blocks-0.1/textures/west.png",
	                        "mods/standard_blocks-0.1/textures/zenith.png", "mods/standard_blocks-0.1/textures/nadir.png"};

	uint32_t width, height;

	{
		std::vector<unsigned char> imageData;
		lodepng::decode(imageData, width, height, skybox[0]);
	}

	uint32_t bufferSize = width * height * 6 * 4;
	auto     buffer     = new Buffer(mDevice.get(), bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE);

	uint32_t offset = 0;
	for (const char* tex : skybox)
	{
		std::vector<unsigned char> imageData;
		lodepng::decode(imageData, width, height, tex);

		buffer->TransferInstantly(imageData.data(), imageData.size(), offset);
		offset += imageData.size();
	}

	{
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType         = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width      = width;
		imageCreateInfo.extent.height     = height;
		imageCreateInfo.extent.depth      = 1;
		imageCreateInfo.mipLevels         = 1;
		imageCreateInfo.arrayLayers       = 6;
		imageCreateInfo.format            = VK_FORMAT_R8G8B8A8_UNORM;
		imageCreateInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage             = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageCreateInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.flags             = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		Texture* skyboxTexture = new Texture(mDevice.get(), mDeviceLocalMemoryHeap.get(), imageCreateInfo, nullptr);
		mResourceManager->RegisterResource<Texture>(skyboxTexture);

		VkBufferImageCopy copies[6] = {};
		copies[0].imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		copies[0].imageSubresource.mipLevel       = 0;
		copies[0].imageSubresource.baseArrayLayer = 0;
		copies[0].imageSubresource.layerCount     = 1;
		copies[0].imageExtent.width               = width;
		copies[0].imageExtent.height              = height;
		copies[0].imageExtent.depth               = 1;
		copies[0].bufferOffset                    = 0;

		for (int i = 1; i < 6; ++i)
		{
			copies[i] = copies[0];
			copies[i].imageSubresource.baseArrayLayer = i;
			copies[i].bufferOffset                    = width * height * 4 * i;
		}

		skyboxTexture->CopyBufferRegionsToImage(buffer, copies, 6);
	}

	delete buffer;
}
