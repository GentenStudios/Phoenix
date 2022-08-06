#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/StaticMesh.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/MemoryHeap.hpp>
#include <Windowing/window.hpp>

#include <SDL.h>
#include <SDL_vulkan.h>

#include <assert.h>
#include <fstream>
#include <iostream>
#include <vector>

#include <string.h>


VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback( VkDebugReportFlagsEXT /*Flags*/, VkDebugReportObjectTypeEXT /*ObjectType*/, uint64_t /*Object*/, size_t /*Location*/, int32_t /*MessageCode*/,
	const char* /*pLayerPrefix*/, const char* pMessage, void* /*pUserData*/ )
{
	std::cout << pMessage << std::endl;
	return VK_FALSE;
}

RenderDevice::RenderDevice( Window* window, uint32_t windowWidth, uint32_t windowHeight ) : mWindowWidth( windowWidth ), mWindowHeight( windowHeight )
{
	volkInitialize( );

	uint32_t supportedPropertiesCount = 0;
	vkEnumerateInstanceExtensionProperties(
		nullptr,
		&supportedPropertiesCount,
		nullptr
	);
	std::vector<VkExtensionProperties> supportedProperties;
	supportedProperties.resize( supportedPropertiesCount );
	vkEnumerateInstanceExtensionProperties(
		nullptr,
		&supportedPropertiesCount,
		supportedProperties.data()
	);

	bool Validation = true;

	const char* instanceExtensions[] = {VK_KHR_SURFACE_EXTENSION_NAME,
	#ifdef _WIN32
	"VK_KHR_win32_surface",
	#elif __linux__
	"VK_KHR_xlib_surface",
	#endif
	"VK_EXT_debug_report"};
	const char* instanceLayers[] = {"VK_LAYER_KHRONOS_validation"};
	

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Prosper";
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pEngineName = "Prosper";
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.apiVersion = VK_MAKE_VERSION( 1, 1, 108 );


	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = 2;
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions; 
	instanceCreateInfo.enabledLayerCount = 0;
	instanceCreateInfo.ppEnabledLayerNames = instanceLayers;

	if ( Validation )
	{
		instanceCreateInfo.enabledExtensionCount += 1;
		instanceCreateInfo.enabledLayerCount += 1;
	}

	Validate(vkCreateInstance(
		&instanceCreateInfo,
		NULL,
		&mInstance
	));

	volkLoadInstance( mInstance );




	if ( Validation )
	{
		PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(
			mInstance,
			"vkCreateDebugReportCallbackEXT"
		));

		VkDebugReportCallbackCreateInfoEXT debugReportCallbackInfo;
		debugReportCallbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		debugReportCallbackInfo.pNext = nullptr;
		debugReportCallbackInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		debugReportCallbackInfo.pfnCallback = &DebugReportCallback;
		debugReportCallbackInfo.pUserData = nullptr;

		// Create the new callback
		Validate( CreateDebugReportCallbackEXT(
			mInstance,
			&debugReportCallbackInfo,
			nullptr,
			&mDebugReportCallback ) );
	}

	if (!SDL_Vulkan_CreateSurface(window->GetWindow(), mInstance, &mSurface))
    {
		assert(0 && "Unable to create surface");
        return;
    }


	const uint32_t requiredDeviceExtentionCount = 2;
	const char* requiredDeviceExtensions[requiredDeviceExtentionCount] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};

	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(
		mInstance,
		&physicalDeviceCount,
		nullptr
	);

	std::unique_ptr<VkPhysicalDevice[]> physicalDevices( new VkPhysicalDevice[physicalDeviceCount]( ) );

	vkEnumeratePhysicalDevices(
		mInstance,
		&physicalDeviceCount,
		physicalDevices.get()
	);

	for ( uint32_t deviceIndex = 0; deviceIndex < physicalDeviceCount; deviceIndex++ )
	{
		if ( HasRequiredExtentions( physicalDevices[deviceIndex], requiredDeviceExtensions, requiredDeviceExtentionCount ) )
		{
			uint32_t queue_family = 0;
			if ( GetQueueFamily( physicalDevices[deviceIndex], VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, queue_family ) )
			{
				VkPhysicalDeviceProperties physicalDeviceProperties;
				vkGetPhysicalDeviceProperties(
					physicalDevices[deviceIndex],
					&physicalDeviceProperties
				);

				VkPhysicalDeviceFeatures physicalDeviceFeatures;
				vkGetPhysicalDeviceFeatures(
					physicalDevices[deviceIndex],
					&physicalDeviceFeatures
				);

				VkPhysicalDeviceMemoryProperties physicalDeviceMemProperties;
				vkGetPhysicalDeviceMemoryProperties(
					physicalDevices[deviceIndex],
					&physicalDeviceMemProperties
				);

				if ( mPhysicalDevice == VK_NULL_HANDLE || mPhysicalDevice != VK_NULL_HANDLE && physicalDeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
				{
					mPhysicalDevice = physicalDevices[deviceIndex];
					mPhysicalDevicesQueueFamily = queue_family;
					mPhysicalDeviceProperties = physicalDeviceProperties;
					mPhysicalDeviceFeatures = physicalDeviceFeatures;
					mPhysicalDeviceMemProperties = physicalDeviceMemProperties;
				}
			}
		}
	}

	if ( mPhysicalDevice == VK_NULL_HANDLE )
	{
		// Error
		assert( 0 && "Unable to get physical device" );
	}

	static const float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = mPhysicalDevicesQueueFamily;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceDescriptorIndexingFeaturesEXT physicalDeviceDescriptorIndexingFeatures{};
	physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
	physicalDeviceDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	physicalDeviceDescriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
	physicalDeviceDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pEnabledFeatures = &mPhysicalDeviceFeatures;
	deviceCreateInfo.enabledExtensionCount = requiredDeviceExtentionCount;
	deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions;
	deviceCreateInfo.pNext = &physicalDeviceDescriptorIndexingFeatures;

	Validate(vkCreateDevice(
		mPhysicalDevice,
		&deviceCreateInfo,
		nullptr,
		&mDevice
	));

	//volkLoadDevice( mDevice );

	vkGetDeviceQueue(
		mDevice,
		mPhysicalDevicesQueueFamily,
		0,
		&mGraphicsQueue
	);

	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = mPhysicalDevicesQueueFamily;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	Validate(vkCreateCommandPool(
		mDevice,
		&pool_info,
		nullptr,
		&mCommandPool
	));


	CreateSwapchain( );

	const uint32_t candidateDepthFormatCount = 3;
	const VkFormat candidateDepthFormats[candidateDepthFormatCount] = {
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	mDepthImageFormat = FindSupportedFormat(
		candidateDepthFormats,
		candidateDepthFormatCount,
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	mSwapchainImageFence = std::unique_ptr<VkFence>( new VkFence[mSwapchainImageCount] );

	for ( uint32_t i = 0; i < mSwapchainImageCount; i++ )
	{
		VkFenceCreateInfo fenceCreateinfo = {};
		fenceCreateinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		Validate( vkCreateFence(
			mDevice,
			&fenceCreateinfo,
			nullptr,
			&mSwapchainImageFence.get( )[i]
		) );
	}


	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	Validate( vkCreateSemaphore(
		mDevice,
		&semaphoreCreateInfo,
		nullptr,
		&mImageAvailableSemaphore
	) );


	Validate( vkCreateSemaphore(
		mDevice,
		&semaphoreCreateInfo,
		nullptr,
		&mRenderFinishedSemaphore
	) );


	mPrimaryCommandBuffers = std::unique_ptr<VkCommandBuffer>( new VkCommandBuffer[mSwapchainImageCount] );

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = mCommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = mSwapchainImageCount;

	Validate( vkAllocateCommandBuffers(
		mDevice,
		&commandBufferAllocateInfo,
		mPrimaryCommandBuffers.get( )
	) );

	mRenderSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	mRenderSubmitInfo.waitSemaphoreCount = 1;
	mRenderSubmitInfo.pWaitDstStageMask = &mRenderWaitStage;
	mRenderSubmitInfo.commandBufferCount = 1;
	mRenderSubmitInfo.signalSemaphoreCount = 1;
	mRenderSubmitInfo.pSignalSemaphores = &mRenderFinishedSemaphore;
	mRenderSubmitInfo.pWaitSemaphores = &mImageAvailableSemaphore;

	mPresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	mPresentInfo.waitSemaphoreCount = 1;
	mPresentInfo.swapchainCount = 1;
	mPresentInfo.pSwapchains = &mSwapChain;
	mPresentInfo.pResults = nullptr;
	mPresentInfo.pWaitSemaphores = &mRenderFinishedSemaphore;

	VkDescriptorSetLayoutBinding samplerDescriptorPoolSizes[] = {
		{ 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 ,VK_SHADER_STAGE_FRAGMENT_BIT }
	};

	mSamplerResourceTableLayout = std::unique_ptr<ResourceTableLayout>( new ResourceTableLayout( this, samplerDescriptorPoolSizes, 1, 100 ) );

}

RenderDevice::~RenderDevice( )
{

	mSamplerResourceTableLayout.reset( );

	vkDestroySemaphore(
		mDevice,
		mImageAvailableSemaphore,
		nullptr
	);

	vkDestroySemaphore(
		mDevice,
		mRenderFinishedSemaphore,
		nullptr
	);

	for ( unsigned int i = 0; i < mSwapchainImageCount; i++ )
	{
		vkDestroyFence(
			mDevice,
			mSwapchainImageFence.get( )[i],
			nullptr
		);
	}

	DestroySwapchain( );

	vkDestroyCommandPool(
		mDevice,
		mCommandPool,
		nullptr
	);

	vkDestroyDevice(
		mDevice,
		nullptr
	);

	vkDestroySurfaceKHR(
		mInstance,
		mSurface,
		nullptr
	);
	
	vkDestroyDebugReportCallbackEXT(
		mInstance,
		mDebugReportCallback,
		nullptr
	);

	vkDestroyInstance(
		mInstance,
		nullptr
	);
}

void RenderDevice::Validate( VkResult res )
{
	switch ( res )
	{
		case VK_SUCCESS: return;
		default:
		{
			assert( 0 );
		}
	}
}

void RenderDevice::CreateImage( uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory, VkImageLayout initialLayout )
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.initialLayout = initialLayout;
	imageCreateInfo.usage = usage;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


	Validate(vkCreateImage(
		mDevice,
		&imageCreateInfo,
		nullptr,
		&image
	));

	VkMemoryRequirements memRequirments;
	vkGetImageMemoryRequirements(
		mDevice,
		image,
		&memRequirments
	);

	uint32_t memoryType = FindMemoryType(
		memRequirments.memoryTypeBits,
		properties
	);

	VkMemoryAllocateInfo memoryAllocationInfo = {};
	memoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocationInfo.allocationSize = memRequirments.size;
	memoryAllocationInfo.memoryTypeIndex = memoryType;

	Validate(vkAllocateMemory(
		mDevice,
		&memoryAllocationInfo,
		nullptr,
		&image_memory
	));

	Validate(vkBindImageMemory(
		mDevice,
		image,
		image_memory,
		0
	));
}

void RenderDevice::TransitionImageLayout( VkCommandBuffer& commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange )
{
	// Define how we will convert between the image layouts
	VkImageMemoryBarrier barrier = ImageMemoryBarrier( image, format, oldLayout, newLayout );

	barrier.subresourceRange = subresourceRange;
	// Submit the barrier update
	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier
	);
}

void RenderDevice::TransitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange )
{
	VkCommandBuffer commandBuffer = CreateSingleTimeCommand( );

	TransitionImageLayout( commandBuffer, image, format, oldLayout, newLayout, subresourceRange );

	// Submit the command to the GPU
	EndCommand( commandBuffer );
}

void RenderDevice::EndCommand( VkCommandBuffer& commandBuffer )
{
	vkEndCommandBuffer( commandBuffer );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	Validate( vkQueueSubmit(
		mGraphicsQueue,
		1,
		&submitInfo,
		VK_NULL_HANDLE
	) );

	vkQueueWaitIdle(
		mGraphicsQueue
	);

	vkFreeCommandBuffers(
		mDevice,
		mCommandPool,
		1,
		&commandBuffer
	);
	commandBuffer = VK_NULL_HANDLE;
}

VkFormat RenderDevice::FindSupportedFormat( const VkFormat* candidateFormats, const uint32_t candidateFormatCount, VkImageTiling tiling, VkFormatFeatureFlags features )
{
	for ( uint32_t i = 0; i < candidateFormatCount; i++ )
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties( mPhysicalDevice, candidateFormats[i], &props );
		if ( tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features )
		{
			return candidateFormats[i];
		}
		else if ( tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features )
		{
			return candidateFormats[i];
		}
	}
	assert( 0 && "All formats are not supported" );
	return VK_FORMAT_UNDEFINED;
}

uint32_t RenderDevice::FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties )
{
	for ( uint32_t i = 0; i < mPhysicalDeviceMemProperties.memoryTypeCount; i++ )
	{
		if ( typeFilter & (1 << i) && (mPhysicalDeviceMemProperties.memoryTypes[i].propertyFlags & properties) == properties )
		{
			return i;
		}
	}
	assert( 0 && "No available memory properties" );
	return UINT32_MAX;
}

uint32_t RenderDevice::FindMemoryType( VkMemoryPropertyFlags properties )
{
	for ( uint32_t i = 0; i < mPhysicalDeviceMemProperties.memoryTypeCount; i++ )
	{
		if ( (mPhysicalDeviceMemProperties.memoryTypes[i].propertyFlags & properties) == properties )
		{
			return i;
		}
	}
	assert( 0 && "No available memory properties" );
	return UINT32_MAX;
}

VkCommandBuffer RenderDevice::CreateSingleTimeCommand( )
{
	VkCommandBufferAllocateInfo commandBufferAllocationInfo = {};
	commandBufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocationInfo.commandPool = mCommandPool;
	commandBufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocationInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	Validate(vkAllocateCommandBuffers(
		mDevice,
		&commandBufferAllocationInfo,
		&commandBuffer
	));	
	
	BeginCommand( commandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT );

	return commandBuffer;
}

VkCommandBuffer RenderDevice::CreateCommand( uint32_t count, VkCommandBufferLevel level )
{
	VkCommandBufferAllocateInfo commandBufferAllocationInfo = {};
	commandBufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocationInfo.commandPool = mCommandPool;
	commandBufferAllocationInfo.level = level;
	commandBufferAllocationInfo.commandBufferCount = count;

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	Validate( vkAllocateCommandBuffers(
		mDevice,
		&commandBufferAllocationInfo,
		&commandBuffer
	) );

	BeginCommand( commandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT );

	return commandBuffer;
}

VkShaderModule RenderDevice::CreateShaderModule( const char* path )
{
	char* shaderData = nullptr;
	unsigned int shaderSize= 0;
	// Load the fragment shader
	ReadBinaryFile(
		path,
		shaderData,
		shaderSize
	);
	
	return CreateShaderModule( shaderData, shaderSize );
}

VkShaderModule RenderDevice::CreateShaderModule( char* data, uint32_t size )
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = size;
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(data);

	VkShaderModule shaderModule = VK_NULL_HANDLE;

	Validate( vkCreateShaderModule(
		mDevice,
		&shaderModuleCreateInfo,
		nullptr,
		&shaderModule
	) );

	return shaderModule;
}

ResourceTableLayout* RenderDevice::GetPostProcessSampler( )
{
	return mSamplerResourceTableLayout.get();
}

void RenderDevice::BeginCommand( VkCommandBuffer commandBuffer, uint32_t flags )
{
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = flags;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	Validate( vkBeginCommandBuffer(
		commandBuffer,
		&commandBufferBeginInfo
	) );

}

void RenderDevice::Present( )
{
	vkWaitForFences(
		mDevice,
		1,
		&mSwapchainImageFence.get( )[mSwapchainImageIndex],
		VK_TRUE,
		UINT32_MAX
	);

	Validate( vkAcquireNextImageKHR(
		mDevice,
		mSwapChain,
		UINT64_MAX,
		mImageAvailableSemaphore,
		VK_NULL_HANDLE,
		&mSwapchainImageIndex
	) );

	Validate( vkQueueWaitIdle(
		mGraphicsQueue
	) );

	Validate( vkResetFences(
		mDevice,
		1,
		&mSwapchainImageFence.get( )[mSwapchainImageIndex]
	) );

	mRenderSubmitInfo.pCommandBuffers = &mPrimaryCommandBuffers.get( )[mSwapchainImageIndex];

	Validate( vkQueueSubmit(
		mGraphicsQueue,
		1,
		&mRenderSubmitInfo,
		mSwapchainImageFence.get( )[mSwapchainImageIndex]
	) );


	mPresentInfo.pImageIndices = &mSwapchainImageIndex;

	Validate( vkQueuePresentKHR(
		mGraphicsQueue,
		&mPresentInfo
	) );

	Validate( vkDeviceWaitIdle( mDevice ) );
	
}

void RenderDevice::WindowChange( uint32_t width, uint32_t height )
{
	mWindowWidth = width;
	mWindowHeight = height;
	DestroySwapchain( );
	CreateSwapchain( );
}

void RenderDevice::CreateSwapchain( )
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::unique_ptr<VkSurfaceFormatKHR[]> surfaceFormats;
	uint32_t surfaceFormatCount = 0;
	std::unique_ptr<VkPresentModeKHR[]> presentModes;
	uint32_t presentModeCount = 0;

	bool hasSupport = CheckSwapchainSupport( capabilities, surfaceFormats, surfaceFormatCount, presentModes, presentModeCount );

	assert( hasSupport );

	mSurfaceFormat = ChooseSwapchainSurfaceFormat( surfaceFormats.get( ), surfaceFormatCount );
	mPresentFormat = ChooseSwapPresentMode( presentModes.get( ), presentModeCount );

	VkExtent2D extent = ChooseSwapExtent( capabilities );

	uint32_t imageCount = capabilities.minImageCount + 1;
	if ( capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount )
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = mSurface;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = mSurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = mSurfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;// | VK_IMAGE_USAGE_STORAGE_BIT; // VK_IMAGE_USAGE_STORAGE_BIT used for raytracing
	swapchainCreateInfo.presentMode = mPresentFormat;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;

	swapchainCreateInfo.preTransform = capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	if ( (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == VK_IMAGE_USAGE_TRANSFER_DST_BIT )
	{
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if ( (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == VK_IMAGE_USAGE_TRANSFER_SRC_BIT )
	{
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	Validate(vkCreateSwapchainKHR(
		mDevice,
		&swapchainCreateInfo,
		nullptr,
		&mSwapChain
	)); 



	Validate( vkGetSwapchainImagesKHR(
		mDevice,
		mSwapChain,
		&mSwapchainImageCount,
		nullptr
	) );

	mSwapchainImages = std::unique_ptr<VkImage>( new VkImage[mSwapchainImageCount] );

	Validate( vkGetSwapchainImagesKHR(
		mDevice,
		mSwapChain,
		&mSwapchainImageCount,
		mSwapchainImages.get()
	) );

	mSwapchainImageViews = std::unique_ptr<VkImageView>( new VkImageView[mSwapchainImageCount] );

	for ( uint32_t i = 0; i < mSwapchainImageCount; i++ )
	{
		TransitionImageLayout( mSwapchainImages.get( )[i], mSurfaceFormat.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1} );

		// Define how the image view will be created
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = mSwapchainImages.get( )[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = mSurfaceFormat.format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		// Create the new image view
		Validate( vkCreateImageView(
			mDevice,
			&imageViewCreateInfo,
			nullptr,
			&mSwapchainImageViews.get( )[i]
		) );
	}
}

void RenderDevice::DestroySwapchain( )
{
	for ( uint32_t i = 0; i < mSwapchainImageCount; i++ )
	{
		vkDestroyImageView(
			mDevice,
			mSwapchainImageViews.get( )[i],
			nullptr
		);
	}
	vkDestroySwapchainKHR(
		mDevice,
		mSwapChain,
		nullptr
	);
}

bool RenderDevice::HasRequiredExtentions( const VkPhysicalDevice& physicalDevice, const char** requiredExtentions, const uint32_t& requiredExtentionCount )
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(
		physicalDevice,
		nullptr,
		&extensionCount,
		nullptr
	);

	std::unique_ptr<VkExtensionProperties[]> availableExtensions( new VkExtensionProperties[extensionCount]( ) );
	vkEnumerateDeviceExtensionProperties(
		physicalDevice,
		nullptr,
		&extensionCount,
		availableExtensions.get()
	);

	for ( uint32_t i = 0; i < requiredExtentionCount; ++i )
	{
		bool extentionFound = false;
		for ( uint32_t j = 0; j < extensionCount; ++j )
		{
			if ( strcmp( requiredExtentions[i], availableExtensions[j].extensionName ) == 0 )
			{
				extentionFound = true;
				break;
			}
		}
		if ( !extentionFound )
		{
			return false;
		}
	}
	return true;
}

bool RenderDevice::GetQueueFamily( const VkPhysicalDevice& physicalDevice, VkQueueFlags requiredQueueFlags, uint32_t& queueFamilyIndex )
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice,
		&queueFamilyCount,
		nullptr
	);

	std::unique_ptr<VkQueueFamilyProperties[]> queueFamilies( new VkQueueFamilyProperties[queueFamilyCount]( ) );
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice,
		&queueFamilyCount,
		queueFamilies.get( )
	);

	for ( uint32_t i = 0; i < queueFamilyCount; ++i )
	{
		if ( queueFamilies[i].queueCount > 0 )
		{
			if ( (queueFamilies[i].queueFlags & requiredQueueFlags) == requiredQueueFlags )
			{

				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(
					physicalDevice,
					i,
					mSurface,
					&presentSupport
				);
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

bool RenderDevice::CheckSwapchainSupport( VkSurfaceCapabilitiesKHR& capabilities, std::unique_ptr<VkSurfaceFormatKHR[]>& formats, uint32_t& formatCount, std::unique_ptr<VkPresentModeKHR[]>& modes, uint32_t& modeCount )
{
	Validate( vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		mPhysicalDevice,
		mSurface,
		&capabilities
	) );


	// Get all formats
	vkGetPhysicalDeviceSurfaceFormatsKHR( mPhysicalDevice, mSurface, &formatCount, nullptr );
	if ( formatCount == 0 ) return false;

	formats = std::unique_ptr<VkSurfaceFormatKHR[]>( new VkSurfaceFormatKHR[formatCount] );

	vkGetPhysicalDeviceSurfaceFormatsKHR( mPhysicalDevice, mSurface, &formatCount, formats.get() );

	// Get all the present modes
	vkGetPhysicalDeviceSurfacePresentModesKHR( mPhysicalDevice, mSurface, &modeCount, nullptr );
	if ( modeCount == 0 ) return false;

	modes = std::unique_ptr<VkPresentModeKHR[]>(new VkPresentModeKHR[modeCount]);

	vkGetPhysicalDeviceSurfacePresentModesKHR( mPhysicalDevice, mSurface, &modeCount, modes.get() );

	return true;
}

VkSurfaceFormatKHR RenderDevice::ChooseSwapchainSurfaceFormat( const VkSurfaceFormatKHR* formats, const uint32_t& formatCount )
{
	if ( formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED )
	{
		return{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	for ( uint32_t i = 0; i < formatCount; i++ )
	{
		if ( formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
		{
			return formats[i];
		}
	}

	if ( formatCount > 0 )return formats[0];

	return{VK_FORMAT_UNDEFINED};
}

VkPresentModeKHR RenderDevice::ChooseSwapPresentMode( const VkPresentModeKHR* modes, const uint32_t& modeCount )
{
	const std::vector<VkPresentModeKHR> modePriorityQueue = {
		VK_PRESENT_MODE_MAILBOX_KHR,
		VK_PRESENT_MODE_IMMEDIATE_KHR,
		VK_PRESENT_MODE_FIFO_RELAXED_KHR
	};

	for ( VkPresentModeKHR priorityMode : modePriorityQueue )
	{
		for ( uint32_t i = 0; i < modeCount; i++ )
		{
			if ( modes[i] == priorityMode )
			{
				return priorityMode;
			}
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D RenderDevice::ChooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities )
{
	if ( capabilities.currentExtent.width != UINT32_MAX )
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D extent = { mWindowWidth, mWindowHeight };
		if ( extent.width > capabilities.maxImageExtent.width )extent.width = capabilities.maxImageExtent.width;
		if ( extent.width < capabilities.minImageExtent.width )extent.width = capabilities.minImageExtent.width;
		if ( extent.height > capabilities.maxImageExtent.width )extent.height = capabilities.maxImageExtent.height;
		if ( extent.height < capabilities.minImageExtent.width )extent.height = capabilities.minImageExtent.height;
		return extent;
	}
}

VkImageMemoryBarrier RenderDevice::ImageMemoryBarrier( VkImage& image, VkFormat& format, VkImageLayout& old_layout, VkImageLayout& new_layout )
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.image = image;
	if ( new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if ( format != VK_FORMAT_UNDEFINED && (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) )
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if ( old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
	{
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if ( old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else if ( old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	else if ( old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if ( old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_GENERAL )
	{
		barrier.srcAccessMask = 0;
	}
	return barrier;
}

void RenderDevice::ReadBinaryFile( const char* filename, char*& data, unsigned int& size )
{
	std::ifstream file( filename, std::ios::ate | std::ios::binary );

	if ( !file.is_open( ) )
	{
		throw std::runtime_error( "failed to open file!" );
	}
	size = (unsigned int)file.tellg( );
	data = new char[size];
	file.seekg( 0 );
	file.read( data, size );
	file.close( );
}