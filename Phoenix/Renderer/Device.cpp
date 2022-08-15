#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/MemoryHeap.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/StaticMesh.hpp>
#include <Windowing/Window.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT /*Flags*/, VkDebugReportObjectTypeEXT /*ObjectType*/,
                                                   uint64_t /*Object*/, size_t /*Location*/, int32_t /*MessageCode*/,
                                                   const char* /*pLayerPrefix*/, const char* pMessage, void* /*pUserData*/)
{
	std::cout << pMessage << std::endl;
	return VK_FALSE;
}

RenderDevice::RenderDevice(Window* window, uint32_t windowWidth, uint32_t windowHeight)
    : m_windowWidth(windowWidth), m_windowHeight(windowHeight)
{
	volkInitialize();

	constexpr bool validation = true;
	std::vector<const char*> instanceExtensions;
	std::vector<const char*> instanceLayers;

	// Load required instance extensions.
	uint32_t instanceExtensionCount = 0;
	SDL_Vulkan_GetInstanceExtensions(window->GetWindow(), &instanceExtensionCount, nullptr);
	instanceExtensions.resize(instanceExtensionCount);
	SDL_Vulkan_GetInstanceExtensions(window->GetWindow(), &instanceExtensionCount, instanceExtensions.data());

	if (validation)
	{
		instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
	}

	VkApplicationInfo appInfo  = {};
	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "Phoenix";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName        = "Phoenix";
	appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion         = VK_MAKE_VERSION(1, 1, 108);

	VkInstanceCreateInfo instanceCreateInfo    = {};
	instanceCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo        = &appInfo;
	instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(instanceExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	instanceCreateInfo.enabledLayerCount       = static_cast<uint32_t>(instanceLayers.size());
	instanceCreateInfo.ppEnabledLayerNames     = instanceLayers.data();

	Validate(vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance));

	volkLoadInstance(m_instance);

	if (validation)
		SetupDebugReportCallback();

	if (!SDL_Vulkan_CreateSurface(window->GetWindow(), m_instance, &m_surface))
	{
		assert(0 && "Unable to create surface");
		return;
	}

	SelectAndCreateDevice();

	// Get the device queue we want to use.
	vkGetDeviceQueue(m_device, m_physicalDevicesQueueFamily, 0, &m_graphicsQueue);

	CreateCommandPools();

	CreateSwapchain();

	const uint32_t candidateDepthFormatCount                        = 3;
	const VkFormat candidateDepthFormats[candidateDepthFormatCount] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
	                                                                   VK_FORMAT_D24_UNORM_S8_UINT};

	m_depthImageFormat = FindSupportedFormat(candidateDepthFormats, candidateDepthFormatCount, VK_IMAGE_TILING_OPTIMAL,
	                                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	CreateSwapchainSyncPrimitives();

	CreatePrimaryCommandBuffers();

	m_renderSubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	m_renderSubmitInfo.waitSemaphoreCount   = 1;
	m_renderSubmitInfo.pWaitDstStageMask    = &m_renderWaitStage;
	m_renderSubmitInfo.commandBufferCount   = 1;
	m_renderSubmitInfo.signalSemaphoreCount = 1;
	m_renderSubmitInfo.pSignalSemaphores    = &m_renderFinishedSemaphore;
	m_renderSubmitInfo.pWaitSemaphores      = &m_imageAvailableSemaphore;

	m_presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	m_presentInfo.waitSemaphoreCount = 1;
	m_presentInfo.swapchainCount     = 1;
	m_presentInfo.pSwapchains        = &m_swapchain;
	m_presentInfo.pResults           = nullptr;
	m_presentInfo.pWaitSemaphores    = &m_renderFinishedSemaphore;

	VkDescriptorSetLayoutBinding samplerDescriptorPoolSizes[] = {
	    {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};

	m_samplerResourceTableLayout = std::make_unique<ResourceTableLayout>(this, samplerDescriptorPoolSizes, 1, 100);
}

RenderDevice::~RenderDevice()
{
	m_samplerResourceTableLayout.reset();

	DestroySwapchainSyncPrimitives();
	DestroySwapchain();

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);

	vkDestroyDevice(m_device, nullptr);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	vkDestroyDebugReportCallbackEXT(m_instance, m_debugReportCallback, nullptr);

	vkDestroyInstance(m_instance, nullptr);
}

void RenderDevice::Validate(VkResult result) const
{
	if (result == VK_SUCCESS)
		return;

	assert(0);
}

void RenderDevice::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                               VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout initialLayout)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType         = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width      = width;
	imageCreateInfo.extent.height     = height;
	imageCreateInfo.extent.depth      = 1;
	imageCreateInfo.mipLevels         = 1;
	imageCreateInfo.arrayLayers       = 1;
	imageCreateInfo.format            = format;
	imageCreateInfo.tiling            = tiling;
	imageCreateInfo.initialLayout     = initialLayout;
	imageCreateInfo.usage             = usage;
	imageCreateInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;

	Validate(vkCreateImage(m_device, &imageCreateInfo, nullptr, &image));

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(m_device, image, &memoryRequirements);

	const uint32_t memoryType = FindMemoryType(memoryRequirements.memoryTypeBits, properties);

	VkMemoryAllocateInfo memoryAllocationInfo = {};
	memoryAllocationInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocationInfo.allocationSize       = memoryRequirements.size;
	memoryAllocationInfo.memoryTypeIndex      = memoryType;

	Validate(vkAllocateMemory(m_device, &memoryAllocationInfo, nullptr, &imageMemory));

	Validate(vkBindImageMemory(m_device, image, imageMemory, 0));
}

void RenderDevice::TransitionImageLayout(VkCommandBuffer& commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout,
                                         VkImageLayout newLayout, VkImageSubresourceRange subresourceRange)
{
	// Define how we will convert between the image layouts
	VkImageMemoryBarrier barrier = ImageMemoryBarrier(image, format, oldLayout, newLayout);
	barrier.subresourceRange     = subresourceRange;

	// Submit the barrier update
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
	                     &barrier);
}

void RenderDevice::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                         VkImageSubresourceRange subresourceRange)
{
	VkCommandBuffer commandBuffer = CreateSingleTimeCommand();
	TransitionImageLayout(commandBuffer, image, format, oldLayout, newLayout, subresourceRange);
	EndCommand(commandBuffer);
}

void RenderDevice::EndCommand(VkCommandBuffer& commandBuffer) const
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo       = {};
	submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &commandBuffer;

	Validate(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
	commandBuffer = VK_NULL_HANDLE;
}

VkFormat RenderDevice::FindSupportedFormat(const VkFormat* candidateFormats, const uint32_t candidateFormatCount, VkImageTiling tiling,
                                           VkFormatFeatureFlags features) const
{
	for (uint32_t i = 0; i < candidateFormatCount; i++)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, candidateFormats[i], &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return candidateFormats[i];
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return candidateFormats[i];
		}
	}

	assert(0 && "All formats are not supported");
	return VK_FORMAT_UNDEFINED;
}

uint32_t RenderDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
	for (uint32_t i = 0; i < m_physicalDeviceMemProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (m_physicalDeviceMemProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	assert(0 && "No available memory properties");
	return UINT32_MAX;
}

uint32_t RenderDevice::FindMemoryType(VkMemoryPropertyFlags properties) const
{
	for (uint32_t i = 0; i < m_physicalDeviceMemProperties.memoryTypeCount; i++)
	{
		if ((m_physicalDeviceMemProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	assert(0 && "No available memory properties");
	return UINT32_MAX;
}

VkCommandBuffer RenderDevice::CreateSingleTimeCommand()
{
	VkCommandBufferAllocateInfo commandBufferAllocationInfo = {};
	commandBufferAllocationInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocationInfo.commandPool                 = m_commandPool;
	commandBufferAllocationInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocationInfo.commandBufferCount          = 1;

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	Validate(vkAllocateCommandBuffers(m_device, &commandBufferAllocationInfo, &commandBuffer));

	BeginCommand(commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	return commandBuffer;
}

VkCommandBuffer RenderDevice::CreateCommand(uint32_t count, VkCommandBufferLevel level)
{
	VkCommandBufferAllocateInfo commandBufferAllocationInfo = {};
	commandBufferAllocationInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocationInfo.commandPool                 = m_commandPool;
	commandBufferAllocationInfo.level                       = level;
	commandBufferAllocationInfo.commandBufferCount          = count;

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	Validate(vkAllocateCommandBuffers(m_device, &commandBufferAllocationInfo, &commandBuffer));

	BeginCommand(commandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

	return commandBuffer;
}

VkShaderModule RenderDevice::CreateShaderModule(const char* path)
{
	char*        shaderData = nullptr;
	unsigned int shaderSize = 0;

	// Load the fragment shader
	ReadBinaryFile(path, shaderData, shaderSize);

	return CreateShaderModule(shaderData, shaderSize);
}

VkShaderModule RenderDevice::CreateShaderModule(char* data, uint32_t size)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize                 = size;
	shaderModuleCreateInfo.pCode                    = reinterpret_cast<const uint32_t*>(data);

	VkShaderModule shaderModule = VK_NULL_HANDLE;

	Validate(vkCreateShaderModule(m_device, &shaderModuleCreateInfo, nullptr, &shaderModule));

	return shaderModule;
}

ResourceTableLayout* RenderDevice::GetPostProcessSampler() const { return m_samplerResourceTableLayout.get(); }

void RenderDevice::BeginCommand(VkCommandBuffer commandBuffer, uint32_t flags) const
{
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags                    = flags;
	commandBufferBeginInfo.pInheritanceInfo         = nullptr;

	Validate(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
}

void RenderDevice::Present()
{
	Validate(vkWaitForFences(m_device, 1, &m_swapchainImageFences[m_swapchainImageIndex], VK_TRUE, UINT32_MAX));

	Validate(vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &m_swapchainImageIndex));

	Validate(vkQueueWaitIdle(m_graphicsQueue));

	Validate(vkResetFences(m_device, 1, &m_swapchainImageFences[m_swapchainImageIndex]));

	m_renderSubmitInfo.pCommandBuffers = &m_primaryCommandBuffers[m_swapchainImageIndex];

	Validate(vkQueueSubmit(m_graphicsQueue, 1, &m_renderSubmitInfo, m_swapchainImageFences[m_swapchainImageIndex]));

	m_presentInfo.pImageIndices = &m_swapchainImageIndex;

	Validate(vkQueuePresentKHR(m_graphicsQueue, &m_presentInfo));
	Validate(vkDeviceWaitIdle(m_device));
}

void RenderDevice::WindowChange(uint32_t width, uint32_t height)
{
	m_windowWidth  = width;
	m_windowHeight = height;

	DestroySwapchain();
	CreateSwapchain();
}

void RenderDevice::SetupDebugReportCallback()
{
	const auto CreateDebugReportCallbackEXT =
	    reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT"));

	VkDebugReportCallbackCreateInfoEXT debugReportCallbackInfo = {};
	debugReportCallbackInfo.sType                              = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debugReportCallbackInfo.pNext                              = nullptr;
	debugReportCallbackInfo.flags =
	    VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	debugReportCallbackInfo.pfnCallback = &DebugReportCallback;
	debugReportCallbackInfo.pUserData   = nullptr;

	// Create the new callback
	Validate(CreateDebugReportCallbackEXT(m_instance, &debugReportCallbackInfo, nullptr, &m_debugReportCallback));
}

void RenderDevice::SelectAndCreateDevice()
{
	const uint32_t requiredDeviceExtensionCount                           = 2;
	const char*    requiredDeviceExtensions[requiredDeviceExtensionCount] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                                          VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};

	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);

	auto physicalDevices = std::make_unique<VkPhysicalDevice[]>(physicalDeviceCount);
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.get());

	for (uint32_t deviceIndex = 0; deviceIndex < physicalDeviceCount; ++deviceIndex)
	{
		if (HasRequiredExtensions(physicalDevices[deviceIndex], requiredDeviceExtensions, requiredDeviceExtensionCount))
		{
			uint32_t queueFamily = 0;
			if (GetQueueFamily(physicalDevices[deviceIndex], VK_QUEUE_GRAPHICS_BIT, queueFamily))
			{
				VkPhysicalDeviceProperties physicalDeviceProperties;
				vkGetPhysicalDeviceProperties(physicalDevices[deviceIndex], &physicalDeviceProperties);

				VkPhysicalDeviceFeatures physicalDeviceFeatures;
				vkGetPhysicalDeviceFeatures(physicalDevices[deviceIndex], &physicalDeviceFeatures);

				VkPhysicalDeviceMemoryProperties physicalDeviceMemProperties;
				vkGetPhysicalDeviceMemoryProperties(physicalDevices[deviceIndex], &physicalDeviceMemProperties);

				if (m_physicalDevice == VK_NULL_HANDLE ||
				    m_physicalDevice != VK_NULL_HANDLE && physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					m_physicalDevice              = physicalDevices[deviceIndex];
					m_physicalDevicesQueueFamily  = queueFamily;
					m_physicalDeviceProperties    = physicalDeviceProperties;
					m_physicalDeviceFeatures      = physicalDeviceFeatures;
					m_physicalDeviceMemProperties = physicalDeviceMemProperties;
				}
			}
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
	{
		// Error
		assert(0 && "Unable to get physical device");
	}

	const float      queuePriority   = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex        = m_physicalDevicesQueueFamily;
	queueCreateInfo.queueCount              = 1;
	queueCreateInfo.pQueuePriorities        = &queuePriority;

	VkPhysicalDeviceDescriptorIndexingFeaturesEXT physicalDeviceDescriptorIndexingFeatures {};
	physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
	physicalDeviceDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	physicalDeviceDescriptorIndexingFeatures.runtimeDescriptorArray                    = VK_TRUE;
	physicalDeviceDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount  = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo      = {};
	deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos       = &queueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount    = 1;
	deviceCreateInfo.pEnabledFeatures        = &m_physicalDeviceFeatures;
	deviceCreateInfo.enabledExtensionCount   = requiredDeviceExtensionCount;
	deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions;
	deviceCreateInfo.pNext                   = &physicalDeviceDescriptorIndexingFeatures;

	Validate(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device));
}

void RenderDevice::CreateCommandPools()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex        = m_physicalDevicesQueueFamily;
	poolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	Validate(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
}

void RenderDevice::CreateSwapchainSyncPrimitives()
{
	m_swapchainImageFences = std::make_unique<VkFence[]>(m_swapchainImageCount);

	for (uint32_t i = 0; i < m_swapchainImageCount; i++)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

		Validate(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_swapchainImageFences[i]));
	}

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	Validate(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_imageAvailableSemaphore));
	Validate(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderFinishedSemaphore));
}

void RenderDevice::CreatePrimaryCommandBuffers()
{
	m_primaryCommandBuffers = std::make_unique<VkCommandBuffer[]>(m_swapchainImageCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool                 = m_commandPool;
	commandBufferAllocateInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount          = m_swapchainImageCount;

	Validate(vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, m_primaryCommandBuffers.get()));
}

void RenderDevice::DestroySwapchainSyncPrimitives() const
{
	vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr);

	for (uint32_t i = 0; i < m_swapchainImageCount; i++)
	{
		vkDestroyFence(m_device, m_swapchainImageFences[i], nullptr);
	}
}

void RenderDevice::CreateSwapchain()
{
	VkSurfaceCapabilitiesKHR              capabilities;
	std::unique_ptr<VkSurfaceFormatKHR[]> surfaceFormats;
	uint32_t                              surfaceFormatCount = 0;
	std::unique_ptr<VkPresentModeKHR[]>   presentModes;
	uint32_t                              presentModeCount = 0;

	const bool hasSupport = CheckSwapchainSupport(capabilities, surfaceFormats, surfaceFormatCount, presentModes, presentModeCount);

	assert(hasSupport);

	m_surfaceFormat = ChooseSwapchainSurfaceFormat(surfaceFormats.get(), surfaceFormatCount);
	m_presentFormat = ChooseSwapPresentMode(presentModes.get(), presentModeCount);

	const VkExtent2D extent = ChooseSwapExtent(capabilities);

	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface                  = m_surface;
	swapchainCreateInfo.minImageCount            = imageCount;
	swapchainCreateInfo.imageFormat              = m_surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace          = m_surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent              = extent;
	swapchainCreateInfo.imageArrayLayers         = 1;
	swapchainCreateInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.presentMode              = m_presentFormat;
	swapchainCreateInfo.clipped                  = VK_TRUE;
	swapchainCreateInfo.oldSwapchain             = VK_NULL_HANDLE;

	swapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices   = nullptr;

	swapchainCreateInfo.preTransform   = capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	if ((capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	{
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if ((capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
	{
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	Validate(vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain));
	Validate(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainImageCount, nullptr));

	m_swapchainImages     = std::make_unique<VkImage[]>(m_swapchainImageCount);
	Validate(vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainImageCount, m_swapchainImages.get()));

	m_swapchainImageViews = std::make_unique<VkImageView[]>(m_swapchainImageCount);

	for (uint32_t i = 0; i < m_swapchainImageCount; i++)
	{
		TransitionImageLayout(m_swapchainImages[i], m_surfaceFormat.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		                      {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

		// Define how the image view will be created
		VkImageViewCreateInfo imageViewCreateInfo           = {};
		imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image                           = m_swapchainImages[i];
		imageViewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format                          = m_surfaceFormat.format;
		imageViewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_R;
		imageViewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_G;
		imageViewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_B;
		imageViewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_A;
		imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
		imageViewCreateInfo.subresourceRange.levelCount     = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount     = 1;

		// Create the new image view
		Validate(vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_swapchainImageViews[i]));
	}
}

void RenderDevice::DestroySwapchain() const
{
	for (uint32_t i = 0; i < m_swapchainImageCount; i++)
	{
		vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

bool RenderDevice::HasRequiredExtensions(const VkPhysicalDevice& physicalDevice, const char** requiredExtensions,
                                         const uint32_t& requiredExtensionCount)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	auto availableExtensions = std::make_unique<VkExtensionProperties[]>(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.get());

	for (uint32_t i = 0; i < requiredExtensionCount; ++i)
	{
		bool extensionFound = false;
		for (uint32_t j = 0; j < extensionCount; ++j)
		{
			if (strcmp(requiredExtensions[i], availableExtensions[j].extensionName) == 0)
			{
				extensionFound = true;
				break;
			}
		}
		if (!extensionFound)
		{
			return false;
		}
	}
	return true;
}

bool RenderDevice::GetQueueFamily(const VkPhysicalDevice& physicalDevice, VkQueueFlags requiredQueueFlags, uint32_t& queueFamilyIndex) const
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	auto queueFamilies = std::make_unique<VkQueueFamilyProperties[]>(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.get());

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if (queueFamilies[i].queueCount > 0)
		{
			if ((queueFamilies[i].queueFlags & requiredQueueFlags) == requiredQueueFlags)
			{
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_surface, &presentSupport);
				if (presentSupport)
				{
					queueFamilyIndex = i;
					return true;
				}

				return false;
			}
		}
	}

	return false;
}

bool RenderDevice::CheckSwapchainSupport(VkSurfaceCapabilitiesKHR& capabilities, std::unique_ptr<VkSurfaceFormatKHR[]>& formats,
                                         uint32_t& formatCount, std::unique_ptr<VkPresentModeKHR[]>& modes, uint32_t& modeCount) const
{
	Validate(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities));

	// Get all formats
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
	if (formatCount == 0)
		return false;

	formats = std::unique_ptr<VkSurfaceFormatKHR[]>(new VkSurfaceFormatKHR[formatCount]);

	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.get());

	// Get all the present modes
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &modeCount, nullptr);
	if (modeCount == 0)
		return false;

	modes = std::unique_ptr<VkPresentModeKHR[]>(new VkPresentModeKHR[modeCount]);

	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &modeCount, modes.get());

	return true;
}

VkSurfaceFormatKHR RenderDevice::ChooseSwapchainSurfaceFormat(const VkSurfaceFormatKHR* formats, const uint32_t& formatCount)
{
	if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	for (uint32_t i = 0; i < formatCount; i++)
	{
		if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return formats[i];
		}
	}

	if (formatCount > 0)
		return formats[0];

	return {VK_FORMAT_UNDEFINED};
}

VkPresentModeKHR RenderDevice::ChooseSwapPresentMode(const VkPresentModeKHR* modes, const uint32_t& modeCount)
{
	const std::vector<VkPresentModeKHR> modePriorityQueue = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR,
	                                                         VK_PRESENT_MODE_FIFO_RELAXED_KHR};

	for (VkPresentModeKHR priorityMode : modePriorityQueue)
	{
		for (uint32_t i = 0; i < modeCount; i++)
		{
			if (modes[i] == priorityMode)
			{
				return priorityMode;
			}
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D RenderDevice::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D extent = {m_windowWidth, m_windowHeight};

		if (extent.width > capabilities.maxImageExtent.width)
			extent.width = capabilities.maxImageExtent.width;
		if (extent.width < capabilities.minImageExtent.width)
			extent.width = capabilities.minImageExtent.width;
		if (extent.height > capabilities.maxImageExtent.width)
			extent.height = capabilities.maxImageExtent.height;
		if (extent.height < capabilities.minImageExtent.width)
			extent.height = capabilities.minImageExtent.height;

		return extent;
	}
}

VkImageMemoryBarrier RenderDevice::ImageMemoryBarrier(VkImage& image, VkFormat& format, VkImageLayout& oldLayout,
                                                      VkImageLayout& newLayout)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.image     = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (format != VK_FORMAT_UNDEFINED && (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = 1;

	if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
	{
		barrier.srcAccessMask = 0;
	}

	return barrier;
}

void RenderDevice::ReadBinaryFile(const char* filename, char*& data, unsigned int& size)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		size = 0;
		data = nullptr;
		return;
	}

	size = static_cast<unsigned>(file.tellg());
	data = new char[size];

	file.seekg(0);
	file.read(data, size);
	file.close();
}
