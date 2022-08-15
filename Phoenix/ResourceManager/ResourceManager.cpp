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
#include <Renderer/Renderpass.hpp>
#include <ResourceManager/RenderTechnique.hpp>
#include <ResourceManager/ResourceManager.hpp>

#include <pugixml.hpp>

#include <assert.h>
#include <sstream>
#include <string.h>

std::string PIPELINES_ROOT = "data/Pipelines/";

ResourceManager::ResourceManager(RenderDevice* device) : mDevice(device) {}

ResourceManager::~ResourceManager()
{
	for (auto& it : mResourceInstances)
	{
		delete it;
	}
	mNamedResourceInstances.clear();
}

void ResourceManager::LoadPipelineDictionary(const char* name, RenderPass* renderPass)
{
	pugi::xml_document     doc;
	pugi::xml_parse_result result = doc.load_file((PIPELINES_ROOT + name).c_str());
	if (!result)
		return;
	pugi::xml_node rootNode = doc.child("Pipelines");

	for (pugi::xml_node pipeline : rootNode.children("Pipeline"))
	{
		LoadPipelineByName(pipeline.attribute("name").as_string(), renderPass);
	}
}

void ResourceManager::LoadPipelineByName(const char* name, RenderPass* renderPass)
{
	RegisterResource(name, new RenderTechnique(mDevice, this, renderPass, name, ((PIPELINES_ROOT + name + ".xml").c_str())), true);
}

