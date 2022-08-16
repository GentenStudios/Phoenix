// BSD 3-Clause License
// 
// Copyright (c) 2022, Genten Studios
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <Renderer/Vulkan.hpp>

#include <memory>
#include <vector>

#ifdef _WIN32
#	include <windows.h>
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
	RenderDevice(Window* window, uint32_t windowWidth, uint32_t windowHeight);
	~RenderDevice();

	void Validate(VkResult result) const;

	VkDevice                         GetDevice() const { return m_device; }
	VkPhysicalDeviceFeatures         GetPhysicalDeviceFeatures() const { return m_physicalDeviceFeatures; }
	VkPhysicalDeviceProperties       GetPhysicalDeviceProperties() const { return m_physicalDeviceProperties; }
	VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemProperties() const { return m_physicalDeviceMemProperties; }

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
	uint32_t FindMemoryType(VkMemoryPropertyFlags properties) const;

	uint32_t     GetWindowWidth() const { return m_windowWidth; }
	uint32_t     GetWindowHeight() const { return m_windowHeight; }
	uint32_t     GetSwapchainImageCount() const { return m_swapchainImageCount; }
	VkImageView* GetSwapchainImageViews() const { return m_swapchainImageViews.get(); }
	VkImage*     GetSwapchainImages() const { return m_swapchainImages.get(); }

	VkFormat GetSurfaceFormat() const { return m_surfaceFormat.format; }
	VkFormat GetColorFormat() const { return m_colorFormat; }
	VkFormat GetDepthFormat() const { return m_depthImageFormat; }

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
	                 VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout initialLayout);

	void TransitionImageLayout(VkCommandBuffer& commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout,
	                           VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);

	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
	                           VkImageSubresourceRange subresourceRange);

	VkFormat FindSupportedFormat(const VkFormat* candidateFormats, const uint32_t candidateFormatCount, VkImageTiling tiling,
	                             VkFormatFeatureFlags features) const;

	VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }

	VkCommandBuffer* GetPrimaryCommandBuffers() const { return m_primaryCommandBuffers.get(); }
	VkCommandBuffer  CreateSingleTimeCommand();
	VkCommandBuffer  CreateCommand(uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	void             BeginCommand(VkCommandBuffer commandBuffer, uint32_t flags) const;
	void             EndCommand(VkCommandBuffer& commandBuffer) const;

	VkShaderModule CreateShaderModule(const char* path);
	VkShaderModule CreateShaderModule(char* data, uint32_t size);

	ResourceTableLayout* GetPostProcessSampler() const;

	void Present();

	void WindowChange(uint32_t width, uint32_t height);

private:
	void SetupDebugReportCallback();
	void SelectAndCreateDevice();
	void CreateCommandPools();
	void CreateSwapchainSyncPrimitives();
	void CreatePrimaryCommandBuffers();

	void DestroySwapchainSyncPrimitives() const;

	void CreateSwapchain();
	void DestroySwapchain() const;

	bool GetQueueFamily(const VkPhysicalDevice& physicalDevice, VkQueueFlags requiredQueueFlags, uint32_t& queueFamilyIndex) const;

	bool CheckSwapchainSupport(VkSurfaceCapabilitiesKHR& capabilities, std::unique_ptr<VkSurfaceFormatKHR[]>& formats,
	                           uint32_t& formatCount, std::unique_ptr<VkPresentModeKHR[]>& modes, uint32_t& modeCount) const;

	static VkSurfaceFormatKHR ChooseSwapchainSurfaceFormat(const VkSurfaceFormatKHR* formats, const uint32_t& formatCount);
	static VkPresentModeKHR   ChooseSwapPresentMode(const VkPresentModeKHR* modes, const uint32_t& modeCount);
	VkExtent2D                ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

	static bool HasRequiredExtensions(const VkPhysicalDevice& physicalDevice, const char** requiredExtensions,
	                                  const uint32_t& requiredExtensionCount);

	static VkImageMemoryBarrier ImageMemoryBarrier(VkImage& image, VkFormat& format, VkImageLayout& oldLayout, VkImageLayout& newLayout);

	static void ReadBinaryFile(const char* filename, char*& data, unsigned int& size);

private:
	VkInstance                       m_instance       = VK_NULL_HANDLE;
	VkPhysicalDevice                 m_physicalDevice = VK_NULL_HANDLE;
	VkDevice                         m_device         = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties       m_physicalDeviceProperties;
	VkPhysicalDeviceFeatures         m_physicalDeviceFeatures;
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemProperties;

	VkSurfaceKHR       m_surface = VK_NULL_HANDLE;
	VkSurfaceFormatKHR m_surfaceFormat;
	VkPresentModeKHR   m_presentFormat;
	VkFormat           m_colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
	VkFormat           m_depthImageFormat;

	uint32_t                       m_windowWidth  = 0;
	uint32_t                       m_windowHeight = 0;
	VkSwapchainKHR                 m_swapchain    = VK_NULL_HANDLE;
	std::unique_ptr<VkImage[]>     m_swapchainImages;
	std::unique_ptr<VkImageView[]> m_swapchainImageViews;
	uint32_t                       m_swapchainImageCount = 0;
	uint32_t                       m_swapchainImageIndex = 0;

	std::unique_ptr<VkFence[]> m_swapchainImageFences;
	VkSemaphore                m_imageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore                m_renderFinishedSemaphore = VK_NULL_HANDLE;

	VkCommandPool m_commandPool                = VK_NULL_HANDLE;
	VkQueue       m_graphicsQueue              = VK_NULL_HANDLE;
	uint32_t      m_physicalDevicesQueueFamily = 0;

	std::unique_ptr<VkCommandBuffer[]> m_primaryCommandBuffers;

	VkPipelineStageFlags m_renderWaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkPresentInfoKHR         m_presentInfo      = {};
	VkSubmitInfo             m_renderSubmitInfo = {};
	VkDebugReportCallbackEXT m_debugReportCallback = nullptr;

	std::vector<BufferTransferRequest> m_memoryTransferRequests;

	std::unique_ptr<ResourceTableLayout> m_samplerResourceTableLayout;
};

