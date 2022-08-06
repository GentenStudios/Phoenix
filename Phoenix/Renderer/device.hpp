#pragma once

#include <Renderer/vulkan.hpp>

#include <memory>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

class StaticMesh;
class DeviceMemory;
class ResourceTableLayout;
class MemoryHeap;
class Window;

struct BufferTransferRequest
{
	VkBuffer dst;
	VkBuffer src;

	uint32_t size;
};

class RenderDevice
{

public:
	RenderDevice( Window* window, uint32_t windowWidth, uint32_t windowHeight );
	~RenderDevice( );

	void Validate( VkResult res );

	void CreateImage( uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
		VkImage& image, VkDeviceMemory& image_memory, VkImageLayout initialLayout );

	void TransitionImageLayout( VkCommandBuffer& commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange );

	void TransitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange );

	void EndCommand( VkCommandBuffer& commandBuffer );

	VkFormat FindSupportedFormat( const VkFormat* candidateFormats, const uint32_t candidateFormatCount, VkImageTiling tiling, VkFormatFeatureFlags features );

	uint32_t FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties );

	uint32_t FindMemoryType( VkMemoryPropertyFlags properties );

	VkFormat GetSurfaceFormat( ) { return mSurfaceFormat.format; }

	VkFormat GetColorFormat( ) { return mColorFormat; }

	VkFormat GetDepthFormat( ) { return mDepthImageFormat; }

	VkDevice GetDevice( ) { return mDevice; }

	uint32_t GetSwapchainImageCount( ) { return mSwapchainImageCount; }

	uint32_t GetWindowWidth( ) { return mWindowWidth; }

	uint32_t GetWindowHeight( ) { return mWindowHeight; }

	VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures( ) { return mPhysicalDeviceFeatures; }

	VkPhysicalDeviceProperties GetPhysicalDeviceProperties( ) { return mPhysicalDeviceProperties; }

	VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemProperties( ) { return mPhysicalDeviceMemProperties; }

	VkImageView* GetSwapchainImageViews( ) { return mSwapchainImageViews.get( ); }

	VkImage* GetSwapchainImages( ) { return mSwapchainImages.get( ); }

	VkCommandBuffer* GetPrimaryCommandBuffers( ) { return mPrimaryCommandBuffers.get(); }

	VkQueue GetGraphicsQueue( ) { return mGraphicsQueue; }
	
	VkCommandBuffer CreateSingleTimeCommand( );

	VkCommandBuffer CreateCommand( uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY );

	VkShaderModule CreateShaderModule( const char* path );

	VkShaderModule CreateShaderModule( char* data, uint32_t size );

	ResourceTableLayout* GetPostProcessSampler( );

	void BeginCommand( VkCommandBuffer commandBuffer, uint32_t flags );

	void Present( );

	void WindowChange( uint32_t width, uint32_t height );

private:
	void CreateSwapchain( );

	void DestroySwapchain( );

	bool HasRequiredExtentions( const VkPhysicalDevice& physicalDevice, const char** requiredExtentions, const uint32_t& requiredExtentionCount );

	bool GetQueueFamily( const VkPhysicalDevice& physicalDevice, VkQueueFlags requiredQueueFlags, uint32_t& queueFamilyIndex );

	bool CheckSwapchainSupport( VkSurfaceCapabilitiesKHR& capabilities, std::unique_ptr<VkSurfaceFormatKHR[]>& formats, uint32_t& format_count, std::unique_ptr<VkPresentModeKHR[]>& modes, uint32_t& mode_count );

	VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat( const VkSurfaceFormatKHR* formats, const uint32_t& formatCount );

	VkPresentModeKHR ChooseSwapPresentMode( const VkPresentModeKHR* modes, const uint32_t& modeCount );

	VkExtent2D ChooseSwapExtent( const VkSurfaceCapabilitiesKHR& capabilities );

	VkImageMemoryBarrier ImageMemoryBarrier( VkImage& image, VkFormat& format, VkImageLayout& old_layout, VkImageLayout& new_layout );

	void ReadBinaryFile( const char* filename, char*& data, unsigned int& size );


	VkInstance mInstance = VK_NULL_HANDLE;
	VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
	VkDevice mDevice = VK_NULL_HANDLE;
	VkSurfaceKHR mSurface = VK_NULL_HANDLE;
	VkCommandPool mCommandPool = VK_NULL_HANDLE;
	VkQueue mGraphicsQueue = VK_NULL_HANDLE;
	VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
	VkSemaphore mImageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore mRenderFinishedSemaphore = VK_NULL_HANDLE;
	std::unique_ptr<VkImage> mSwapchainImages = nullptr;
	std::unique_ptr<VkImageView> mSwapchainImageViews = nullptr;
	std::unique_ptr<VkFence> mSwapchainImageFence = nullptr;
	std::unique_ptr<VkCommandBuffer> mPrimaryCommandBuffers = nullptr;

	uint32_t mWindowWidth = 0;
	uint32_t mWindowHeight = 0;
	uint32_t mPhysicalDevicesQueueFamily = 0;
	uint32_t mSwapchainImageCount = 0;
	uint32_t mSwapchainImageIndex = 0;
	VkPhysicalDeviceProperties mPhysicalDeviceProperties;
	VkPhysicalDeviceFeatures mPhysicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties mPhysicalDeviceMemProperties;

	VkSurfaceFormatKHR mSurfaceFormat;
	VkPresentModeKHR mPresentFormat;
	VkFormat mColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
	VkFormat mDepthImageFormat;

	VkPipelineStageFlags mRenderWaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkPresentInfoKHR mPresentInfo = {};
	VkSubmitInfo mRenderSubmitInfo = {};
	VkDebugReportCallbackEXT mDebugReportCallback;

	std::vector< BufferTransferRequest> mMemoryTransferRequests;

	std::unique_ptr<ResourceTableLayout> mSamplerResourceTableLayout;

};