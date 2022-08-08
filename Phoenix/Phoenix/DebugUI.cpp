#include <Phoenix/DebugUI.hpp>

#include <functional>

#include <Windowing/Window.hpp>

#include <Renderer/Device.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/Buffer.hpp>
#include <Renderer/MemoryHeap.hpp>
#include <Renderer/RenderTarget.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/Renderpass.hpp>
#include <Renderer/Texture.hpp>

#include <ResourceManager/ResourceManager.hpp>


const char* FONT_NAME = "Font";
const uint32_t MAX_VERTICIES = 100000;
const uint32_t MAX_INDEXES = 100000;
const uint32_t MAX_DRAW_CALLS = 1000;
const float FONT_SIZE = 20.0f;


DebugUI::DebugUI(RenderDevice* device, ResourceManager* resourecManager, Window* window) : mDevice(device), mResourceManager(resourecManager)
{
	InitImGui();
	InitWindowCallbacks(window);
	CreateResourceTableLayouts();
	CreateBuffers();
	ViewportResize();
	InitRenderPassResources();
	CreatePipelines();

	mOutOfDateDateCMD = true;
}

DebugUI::~DebugUI()
{
	delete mImGuiConfigurationTableLayout;
	delete mImGuiSamplersTableLayout;
	delete mImGuiConfigurationTable;
	delete mImGuiSamplersTable;
	delete mImGuiPipelineLayout;
	delete mImGuiPipeline;

	delete mImGuiConfigurationBuffer;
	delete mImGuiVetexBuffer;
	delete mImGuiTextureVetexBuffer;
	delete mImGuiIndexBuffer;
	delete mImGuiIndirectDrawBuffer;

	delete mRenderTarget;

	delete mFontTexture;

	for (auto& it : mShaderModules)
	{
		vkDestroyShaderModule(
			mDevice->GetDevice(),
			it.second,
			nullptr
		);
	}
}

void DebugUI::Use(VkCommandBuffer* commandBuffers, uint32_t index, bool useParentRenderTarget)
{
	if (!useParentRenderTarget)
	{
		mRenderTarget->GetRenderPass()->Use(commandBuffers, index);
	}

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = mDevice->GetWindowWidth();
	viewport.height = mDevice->GetWindowHeight();
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.extent.width = mDevice->GetWindowWidth();
	scissor.extent.height = mDevice->GetWindowHeight();
	scissor.offset.x = 0;
	scissor.offset.y = 0;

	vkCmdSetViewport(
		commandBuffers[index],
		0,
		1,
		&viewport
	);

	vkCmdSetScissor(
		commandBuffers[index],
		0,
		1,
		&scissor
	);

	mImGuiPipeline->Use(commandBuffers, index);
	mImGuiSamplersTable->Use(commandBuffers, index, 0, mImGuiPipelineLayout->GetPipelineLayout());
	mImGuiConfigurationTable->Use(commandBuffers, index, 1, mImGuiPipelineLayout->GetPipelineLayout());

	{
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(
			commandBuffers[index],
			0,
			1,
			&mImGuiVetexBuffer->GetBuffer(),
			offsets
		);
	}
	{
		vkCmdBindIndexBuffer(
			commandBuffers[index],
			mImGuiIndexBuffer->GetBuffer(),
			0,
			VK_INDEX_TYPE_UINT32
		);
	}

	for (uint32_t i = 0; i < MAX_DRAW_CALLS; i++)
	{

		vkCmdSetScissor(
			commandBuffers[index],
			0,
			1,
			&mScissors.get()[i]
		);

		VkDeviceSize offsets[] = { i * sizeof(int32_t) };
		vkCmdBindVertexBuffers(
			commandBuffers[index],
			1,
			1,
			&mImGuiTextureVetexBuffer->GetBuffer(),
			offsets
		);

		vkCmdDrawIndexedIndirect(
			commandBuffers[index],
			mImGuiIndirectDrawBuffer->GetBuffer(),
			i * sizeof(VkDrawIndexedIndirectCommand),
			1,
			sizeof(VkDrawIndexedIndirectCommand));
	}

	if (!useParentRenderTarget)
	{
		vkCmdEndRenderPass(
			commandBuffers[index]
		);
	}
}

void DebugUI::Update(float delta)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = delta;

	ImGui::NewFrame();

	
	if (ImGui::BeginMainMenuBar())
	{
		for (auto& callback : mMainMenuCallbacks)
		{
			callback.function(callback.ref);
		}
		ImGui::EndMainMenuBar();
	}

	for (auto& callback : mRenderCallbacks)
	{
		callback.function(callback.ref);
	}

	//ImGui::ShowDemoWindow();

	ImGui::Render();

	ImDrawData* imDrawData = ImGui::GetDrawData();

	ImVec2 clip_off = imDrawData->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = imDrawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
	int fb_width = (int)(imDrawData->DisplaySize.x * imDrawData->FramebufferScale.x);
	int fb_height = (int)(imDrawData->DisplaySize.y * imDrawData->FramebufferScale.y);

	ImDrawVert* temp_vertex_data = mImGuiVetexBufferCPU.get();
	uint32_t* temp_index_data = mImGuiIndexBufferCPU.get();
	int32_t* temp_texture_data = mImGuiTextureVetexBufferCPU.get();
	VkDrawIndexedIndirectCommand* temp_indirect_draw = mImGuiIndirectDrawBufferCPU.get();

	unsigned int index_count = 0;
	unsigned int vertex_count = 0;
	int drawGroup = 0;

	for (uint32_t i = 0; i < MAX_DRAW_CALLS; i++)
	{
		temp_indirect_draw[i].instanceCount = 0;
	}

	mOutOfDateDateCMD = false;

	for (int n = 0; n < imDrawData->CmdListsCount; n++)
	{

		const ImDrawList* cmd_list = imDrawData->CmdLists[n];

		memcpy(temp_vertex_data, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));

		// Loop through and manually add a offset to the index's so they can all be rendered in one render pass
		for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
		{
			temp_vertex_data[i] = cmd_list->VtxBuffer.Data[i];
		}
		for (int i = 0; i < cmd_list->IdxBuffer.Size; i++)
		{
			temp_index_data[i] = cmd_list->IdxBuffer.Data[i];
		}

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];



			// Generate the clip area
			ImVec4 clip_rect;
			clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
			clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
			clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
			clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

			// If the object is out of the scissor, ignore it
			if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
			{
				// Negative offsets are illegal for vkCmdSetScissor
				if (clip_rect.x < 0.0f)
					clip_rect.x = 0.0f;
				if (clip_rect.y < 0.0f)
					clip_rect.y = 0.0f;

				// Generate a vulkan friendly scissor area
				VkRect2D scissor;
				scissor.offset.x = (int32_t)(clip_rect.x);
				scissor.offset.y = (int32_t)(clip_rect.y);
				scissor.extent.width = (uint32_t)(clip_rect.z - clip_rect.x);
				scissor.extent.height = (uint32_t)(clip_rect.w - clip_rect.y);


				if (scissor.offset.x != mScissors.get()[drawGroup].offset.x || scissor.offset.y != mScissors.get()[drawGroup].offset.y ||
					scissor.extent.width != mScissors.get()[drawGroup].extent.width || scissor.extent.height != mScissors.get()[drawGroup].extent.height)
				{
					mOutOfDateDateCMD = true;
					mScissors.get()[drawGroup] = scissor;
				}


				VkDrawIndexedIndirectCommand& indirect_command = temp_indirect_draw[drawGroup];
				indirect_command.indexCount = pcmd->ElemCount;
				indirect_command.instanceCount = 1;
				indirect_command.firstIndex = pcmd->IdxOffset + index_count;
				indirect_command.vertexOffset = pcmd->VtxOffset + vertex_count;
				indirect_command.firstInstance = 0;

				temp_texture_data[drawGroup] = (intptr_t)pcmd->TextureId;

				drawGroup++;
			}
		}

		temp_vertex_data += cmd_list->VtxBuffer.Size;
		temp_index_data += cmd_list->IdxBuffer.Size;

		vertex_count += cmd_list->VtxBuffer.Size;
		index_count += cmd_list->IdxBuffer.Size;
	}

	mImGuiVetexBuffer->TransferInstantly(mImGuiVetexBufferCPU.get(), mImGuiVetexBuffer->GetBufferSize());
	mImGuiTextureVetexBuffer->TransferInstantly(mImGuiTextureVetexBufferCPU.get(), mImGuiTextureVetexBuffer->GetBufferSize());
	mImGuiIndexBuffer->TransferInstantly(mImGuiIndexBufferCPU.get(), mImGuiIndexBuffer->GetBufferSize());
	mImGuiIndirectDrawBuffer->TransferInstantly(mImGuiIndirectDrawBufferCPU.get(), mImGuiIndirectDrawBuffer->GetBufferSize());
}

void DebugUI::ViewportResize()
{
	mImGuiConfiguration.screenWidth = mDevice->GetWindowWidth();
	mImGuiConfiguration.screenHeight = mDevice->GetWindowHeight();

	mImGuiConfigurationBuffer->TransferInstantly(&mImGuiConfiguration, sizeof(ImGuiConfiguration));

	InitRenderPassResources();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(mDevice->GetWindowWidth(), mDevice->GetWindowHeight());
}

void DebugUI::AddRenderCallback(std::function<void(void*)> callback, void* ref)
{
	mRenderCallbacks.push_back({ callback, ref }); }

void DebugUI::AddMainmenuCallback(std::function<void(void*)> callback, void* ref)
{
	mMainMenuCallbacks.push_back({callback, ref});
}

void DebugUI::InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(mDevice->GetWindowWidth(), mDevice->GetWindowHeight());
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable | ImGuiBackendFlags_HasMouseHoveredViewport;

	ImGui::GetStyle().TabRounding = 0;


	io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
	io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
	io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
	io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
	io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
	io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
	io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
	io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
	io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
	io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
	io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
	io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
	io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.42f, 1.00f, 1.00f);
}

void DebugUI::InitWindowCallbacks(Window* window)
{

	std::function<void(SDL_Event&, void*)> mouseButtonEvent = [](SDL_Event& event, void* callback)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (event.button.button == SDL_BUTTON_LEFT) io.MouseDown[0] = event.button.state == SDL_PRESSED;
		if (event.button.button == SDL_BUTTON_RIGHT) io.MouseDown[1] = event.button.state == SDL_PRESSED;
		if (event.button.button == SDL_BUTTON_MIDDLE) io.MouseDown[2] = event.button.state == SDL_PRESSED;
	};

	window->AddEventCallback(SDL_MOUSEBUTTONDOWN, mouseButtonEvent, this);
	window->AddEventCallback(SDL_MOUSEBUTTONUP, mouseButtonEvent, this);

	std::function<void(SDL_Event&, void*)> keyEvent = [](SDL_Event& event, void* callback)
	{
		ImGuiIO& io = ImGui::GetIO();
		int key = event.key.keysym.scancode;
		IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
		{
			io.KeysDown[key] = (event.type == SDL_KEYDOWN);
		}
		io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
		io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
		io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
		io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
	};

	window->AddEventCallback(SDL_KEYDOWN, keyEvent, this);
	window->AddEventCallback(SDL_KEYUP, keyEvent, this);

	window->AddEventCallback(SDL_MOUSEMOTION, [](SDL_Event& event, void* callback)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MousePos = ImVec2(event.motion.x, event.motion.y);
		}, this);

	window->AddEventCallback(SDL_TEXTINPUT, [](SDL_Event& event, void* callback)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddInputCharactersUTF8(event.text.text);
		}, this);

	window->AddEventCallback(SDL_MOUSEWHEEL, [](SDL_Event& event, void* callback)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (event.wheel.x > 0) io.MouseWheelH += 1;
			if (event.wheel.x < 0) io.MouseWheelH -= 1;
			if (event.wheel.y > 0) io.MouseWheel += 1;
			if (event.wheel.y < 0) io.MouseWheel -= 1;
		}, this);
}

void DebugUI::CreateResourceTableLayouts()
{

	{	// ImGui Gpu Resources
		VkDescriptorSetLayoutBinding descriptorPoolSizes[] = {
			   { 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 ,VK_SHADER_STAGE_VERTEX_BIT }
		};
		mImGuiConfigurationTableLayout = new ResourceTableLayout(mDevice, descriptorPoolSizes, 1, 1);

		mImGuiConfigurationTable = mImGuiConfigurationTableLayout->CreateTable();
	}
	{	// ImGui Samplers
		VkDescriptorSetLayoutBinding descriptorPoolSizes[] = {
			   { 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 ,VK_SHADER_STAGE_FRAGMENT_BIT }
		};
		mImGuiSamplersTableLayout = new ResourceTableLayout(mDevice, descriptorPoolSizes, 1, 100);

		mImGuiSamplersTable = mImGuiSamplersTableLayout->CreateTable();
	}
}

void DebugUI::CreateBuffers()
{

	mImGuiVetexBufferCPU = std::unique_ptr<ImDrawVert>(new ImDrawVert[MAX_VERTICIES]);
	mImGuiIndexBufferCPU = std::unique_ptr<uint32_t>(new uint32_t[MAX_INDEXES]);
	mImGuiTextureVetexBufferCPU = std::unique_ptr<int32_t>(new int32_t[MAX_DRAW_CALLS]);
	mImGuiIndirectDrawBufferCPU = std::unique_ptr<VkDrawIndexedIndirectCommand>(new VkDrawIndexedIndirectCommand[MAX_DRAW_CALLS]);


	{
		mScissors = std::unique_ptr<VkRect2D>(new VkRect2D[MAX_DRAW_CALLS]);
		for (uint32_t i = 0; i < MAX_DRAW_CALLS; i++)
		{
			mScissors.get()[i] = {};
		}
	}

	MemoryHeap* GPUMappableMemoryHeap = mResourceManager->GetResource<MemoryHeap>("GPUMappableMemoryHeap");
	MemoryHeap* deviceLocalMemoryHeap = mResourceManager->GetResource<MemoryHeap>("DeviceLocalMemoryHeap");


	{
		// Load font sheets
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* fontData;
		int fontWidth, fontHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &fontWidth, &fontHeight);

		mFontTexture = new Texture(
			mDevice,
			deviceLocalMemoryHeap,
			fontWidth,
			fontHeight,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_SAMPLED_BIT,
			reinterpret_cast<char*>(fontData)
		);

		for (int i = 0; i < 100; i++)
		{
			mImGuiSamplersTable->Bind(0, mFontTexture, i);
		}

		io.Fonts->TexID = (ImTextureID)1;

		delete[] fontData;
	}


	{

		mImGuiConfigurationBuffer = new Buffer(
			mDevice, GPUMappableMemoryHeap, sizeof(ImGuiConfiguration),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE
		);
		mImGuiConfigurationTable->Bind(0, mImGuiConfigurationBuffer);
	}
	{

		mImGuiVetexBuffer = new Buffer(
			mDevice, GPUMappableMemoryHeap, sizeof(ImDrawVert) * MAX_VERTICIES,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE
		);
	}
	{

		mImGuiTextureVetexBuffer = new Buffer(
			mDevice, GPUMappableMemoryHeap, sizeof(int32_t) * MAX_DRAW_CALLS,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE
		);
	}
	{

		mImGuiIndexBuffer = new Buffer(
			mDevice, GPUMappableMemoryHeap, sizeof(uint32_t) * MAX_INDEXES,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE
		);
	}
	{
		mImGuiIndirectDrawBuffer = new Buffer(
			mDevice, GPUMappableMemoryHeap, sizeof(VkDrawIndexedIndirectCommand) * MAX_DRAW_CALLS,
			VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE
		);
	}
}

void DebugUI::InitRenderPassResources()
{
	if (mRenderTarget == nullptr)
	{
		mRenderTarget = new RenderTarget(
			mDevice,
			mDevice->GetWindowWidth(),
			mDevice->GetWindowHeight(),
			true
		);
	}
	else
	{
		mRenderTarget->ScreenResize(
			mDevice->GetWindowWidth(),
			mDevice->GetWindowHeight()
		);
	}
}

void DebugUI::CreatePipelines()
{
	VkDescriptorSetLayout descriptorSetLayouts[2]
	{
		mImGuiSamplersTableLayout->GetDescriptorSetLayout(),
		mImGuiConfigurationTableLayout->GetDescriptorSetLayout()
	};
	mImGuiPipelineLayout = new PipelineLayout(
		mDevice,
		descriptorSetLayouts,
		2
	);


	VkVertexInputBindingDescription vertexInputBindingDescriptions[2];
	{
		// Position, UV, Color
		vertexInputBindingDescriptions[0].binding = 0;
		vertexInputBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexInputBindingDescriptions[0].stride = sizeof(ImDrawVert);

		// Texture ID
		vertexInputBindingDescriptions[1].binding = 1;
		vertexInputBindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
		vertexInputBindingDescriptions[1].stride = sizeof(int);
	}

	VkVertexInputAttributeDescription vertexInputAttributeDescriptions[4];
	{
		// Position
		vertexInputAttributeDescriptions[0].binding = 0;
		vertexInputAttributeDescriptions[0].location = 0;
		vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributeDescriptions[0].offset = offsetof(ImDrawVert, pos);

		// UV
		vertexInputAttributeDescriptions[1].binding = 0;
		vertexInputAttributeDescriptions[1].location = 1;
		vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributeDescriptions[1].offset = offsetof(ImDrawVert, uv);

		// Color
		vertexInputAttributeDescriptions[2].binding = 0;
		vertexInputAttributeDescriptions[2].location = 2;
		vertexInputAttributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		vertexInputAttributeDescriptions[2].offset = offsetof(ImDrawVert, col);

		// Texture ID
		vertexInputAttributeDescriptions[3].binding = 1;
		vertexInputAttributeDescriptions[3].location = 3;
		vertexInputAttributeDescriptions[3].format = VK_FORMAT_R32_SINT;
		vertexInputAttributeDescriptions[3].offset = 0;
	}


	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2];
	{
		const char* VSPath = "data/Shaders/ImGUI/vert.spv";
		const char* FSPath = "data/Shaders/ImGUI/frag.spv";
		mShaderModules[VSPath] = mDevice->CreateShaderModule(VSPath);
		pipelineShaderStageCreateInfos[0] = {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT,
			mShaderModules[VSPath],
			"main"
		};
		mShaderModules[FSPath] = mDevice->CreateShaderModule(FSPath);
		pipelineShaderStageCreateInfos[1] = {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			mShaderModules[FSPath],
			"main"
		};
	}

	mImGuiPipeline = new Pipeline(
		mDevice, PipelineType::Graphics, mRenderTarget->GetRenderPass(), mImGuiPipelineLayout,
		pipelineShaderStageCreateInfos, 2,
		vertexInputBindingDescriptions, 2,
		vertexInputAttributeDescriptions, 4,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	);
}
