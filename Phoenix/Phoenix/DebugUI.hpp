#pragma once

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <vector>

#include <imgui/imgui.h>

#include <Renderer/Vulkan.hpp>

class RenderDevice;
class ResourceManager;
class Window;
class RenderTarget;
class ResourceTable;
class PipelineLayout;
class Pipeline;
class ResourceTableLayout;
class Buffer;
class Texture;

class DebugUI
{
	struct ImGuiConfiguration
	{
		float screenWidth;
		float screenHeight;
	};
	struct RenderCallback
	{
		std::function<void(void*)> function;
		void* ref;
	};

public:
	DebugUI(RenderDevice* device, ResourceManager* resourecManager, Window* window);

	~DebugUI();

	void Use(VkCommandBuffer* commandBuffer, uint32_t index, bool useParentRenderTarget);

	void Update(float delta);

	bool IsCMDOutdated() { return mOutOfDateDateCMD; }

	void ViewportResize();

	void AddRenderCallback(std::function<void(void*)> callback, void* ref);

	void AddMainmenuCallback(std::function<void(void*)> callback, void* ref);

private:

	void InitImGui();

	void InitWindowCallbacks(Window* window);

	void CreateResourceTableLayouts();

	void CreateBuffers();

	void InitRenderPassResources();

	void CreatePipelines();

	RenderDevice* mDevice;
	RenderTarget* mRenderTarget = nullptr;
	ResourceManager* mResourceManager;

	bool mOutOfDateDateCMD = false;

	ResourceTableLayout* mImGuiConfigurationTableLayout = nullptr;
	ResourceTableLayout* mImGuiSamplersTableLayout = nullptr;
	ResourceTable* mImGuiConfigurationTable = nullptr;
	ResourceTable* mImGuiSamplersTable = nullptr;
	PipelineLayout* mImGuiPipelineLayout = nullptr;
	Pipeline* mImGuiPipeline = nullptr;

	ImGuiConfiguration mImGuiConfiguration;
	Buffer* mImGuiConfigurationBuffer = nullptr;
	Buffer* mImGuiVetexBuffer = nullptr;
	Buffer* mImGuiTextureVetexBuffer = nullptr;
	Buffer* mImGuiIndexBuffer = nullptr;
	Buffer* mImGuiIndirectDrawBuffer = nullptr;

	std::unique_ptr<ImDrawVert> mImGuiVetexBufferCPU;
	std::unique_ptr<uint32_t> mImGuiIndexBufferCPU;
	std::unique_ptr<int32_t> mImGuiTextureVetexBufferCPU;
	std::unique_ptr<VkDrawIndexedIndirectCommand> mImGuiIndirectDrawBufferCPU;

	Texture* mFontTexture = nullptr;

	std::vector<RenderCallback> mRenderCallbacks;
	std::vector<RenderCallback> mMainMenuCallbacks;

	std::unique_ptr<VkRect2D> mScissors;
	std::map<std::string, VkShaderModule> mShaderModules;

};