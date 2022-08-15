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

#include <Renderer/Device.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/RenderTarget.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <ResourceManager/RenderTechnique.hpp>
#include <ResourceManager/ResourceManager.hpp>

#include <pugixml.hpp>

#include <assert.h>
#include <string.h>

const uint32_t MAX_INPUT_BINDINGS   = 100;
const uint32_t MAX_INPUT_ATTRIBUTES = 100;
const uint32_t MAX_DESCRIPTOR_SETS  = 32;

RenderTechnique::RenderTechnique(RenderDevice* device, ResourceManager* resourceManager, RenderPass* renderPass, const char* name,
                                 const char* path)
    : mDevice(device), mResourceManager(resourceManager), mName(name), mPath(path)
{
	pugi::xml_document     doc;
	pugi::xml_parse_result result = doc.load_file(path);
	if (!result)
		return;
	pugi::xml_node rootNode     = doc.child("Pipeline");
	std::string    pipelineType = rootNode.attribute("type").as_string();
	std::string    topologyType = rootNode.attribute("topology").as_string();

	pugi::xml_node descriptorNode      = rootNode.child("Descriptors");
	pugi::xml_node stagesNode          = rootNode.child("Stages");
	pugi::xml_node bindingsNode        = rootNode.child("VertexBindings");
	pugi::xml_node inputAttributesNode = rootNode.child("VertexInputAttributes");

	VkDescriptorSetLayout descriptorSetLayouts[MAX_DESCRIPTOR_SETS];
	uint32_t              descriptorCount = 0;

	VkVertexInputBindingDescription vertexInputBindingDescriptions[MAX_INPUT_BINDINGS];
	uint32_t                        bindingCount = 0;

	VkVertexInputAttributeDescription vertexInputAttributeDescriptions[MAX_INPUT_ATTRIBUTES];
	uint32_t                          attributeCount = 0;

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[MAX_SHADER_MODULES];

	for (pugi::xml_node descriptor : descriptorNode.children("Descriptor"))
	{
		descriptorSetLayouts[descriptorCount++] =
		    resourceManager->GetResource<ResourceTableLayout>(descriptor.attribute("name").as_string())->GetDescriptorSetLayout();
	}

	for (pugi::xml_node stage : stagesNode.children("Stage"))
	{
		mShaderModule[stageCount]                  = mDevice->CreateShaderModule(stage.attribute("path").as_string());
		pipelineShaderStageCreateInfos[stageCount] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		                                              nullptr,
		                                              0,
		                                              GetStageFromAttribute(stage.attribute("stage").as_string()),
		                                              mShaderModule[stageCount],
		                                              stage.attribute("entrypoint").as_string()};

		stageCount++;
	}

	for (pugi::xml_node binding : bindingsNode.children("Binding"))
	{
		vertexInputBindingDescriptions[bindingCount++] = {binding.attribute("binding").as_uint(), binding.attribute("stride").as_uint(),
		                                                  GetVertexInputFromAttribute(binding.attribute("rate").as_string())};
	}

	for (pugi::xml_node attribute : inputAttributesNode.children("Attribute"))
	{
		vertexInputAttributeDescriptions[attributeCount++] = {
		    attribute.attribute("location").as_uint(), attribute.attribute("binding").as_uint(),
		    GetFormatFromAttribute(attribute.attribute("format").as_string()), attribute.attribute("offset").as_uint()};
	}

	mPipelineLayout = new PipelineLayout(mDevice, descriptorSetLayouts, descriptorCount);

	PipelineType        type;
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	if (pipelineType == "Graphics")
	{
		type = PipelineType::Graphics;

		if (topologyType == "Triangle")
		{
			topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
		else if (topologyType == "Point")
		{
			topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		}
		else
		{
			assert(0 && "MISSING TPOLOGY");
		}
	}
	else if (pipelineType == "Compute")
	{
		type = PipelineType::Compute;
	}
	else
	{
		assert(0 && "MISSING STAGE");
	}

	mPipeline = new Pipeline(mDevice, type, renderPass, mPipelineLayout, pipelineShaderStageCreateInfos, stageCount,
	                         vertexInputBindingDescriptions, bindingCount, vertexInputAttributeDescriptions, attributeCount, topology);
}

RenderTechnique::~RenderTechnique()
{
	for (uint32_t i = 0; i < stageCount; i++)
	{
		vkDestroyShaderModule(mDevice->GetDevice(), mShaderModule[i], nullptr);
	}

	delete mPipeline;

	delete mPipelineLayout;
}

VkVertexInputRate RenderTechnique::GetVertexInputFromAttribute(const char* text)
{
	if (strcmp(text, "INPUT_RATE_VERTEX") == 0)
	{
		return VK_VERTEX_INPUT_RATE_VERTEX;
	}
	else if (strcmp(text, "INPUT_RATE_INSTANCE") == 0)
	{
		return VK_VERTEX_INPUT_RATE_INSTANCE;
	}
	return VK_VERTEX_INPUT_RATE_MAX_ENUM;
}

VkFormat RenderTechnique::GetFormatFromAttribute(const char* text)
{
	if (strcmp(text, "R32G32_SFLOAT") == 0)
	{
		return VK_FORMAT_R32G32_SFLOAT;
	}
	if (strcmp(text, "R32G32B32_SFLOAT") == 0)
	{
		return VK_FORMAT_R32G32B32_SFLOAT;
	}
	if (strcmp(text, "R32G32B32A32_SFLOAT") == 0)
	{
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	if (strcmp(text, "R32_UINT") == 0)
	{
		return VK_FORMAT_R32_UINT;
	}
	if (strcmp(text, "R32_SINT") == 0)
	{
		return VK_FORMAT_R32_SINT;
	}

	assert(0 && "MISSING FORMAT");
	return VK_FORMAT_MAX_ENUM;
}

VkShaderStageFlagBits RenderTechnique::GetStageFromAttribute(const char* text)
{
	if (strcmp(text, "Vertex") == 0)
	{
		return VK_SHADER_STAGE_VERTEX_BIT;
	}
	else if (strcmp(text, "Geometry") == 0)
	{
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	}
	else if (strcmp(text, "Fragment") == 0)
	{
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	else if (strcmp(text, "Compute") == 0)
	{
		return VK_SHADER_STAGE_COMPUTE_BIT;
	}
	assert(0 && "MISSING STAGE");
	return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}
