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
