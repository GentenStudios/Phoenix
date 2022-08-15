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

#include <Renderer/MemoryHeap.hpp>
#include <Renderer/ResourcePacket.hpp>

#include <assert.h>
#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

class RenderDevice;
class DeviceMemory;
class ResourcePacketInterface;
class ResourceTableLayout;
class RenderTechnique;
class RenderPass;

class ResourceManager
{
public:
	ResourceManager(RenderDevice* device);
	~ResourceManager();

	template <typename T>
	void RegisterResource(std::string name, T* t, bool autoCleanup = true);

	template <typename T>
	void RegisterResource(T* t, bool autoCleanup = true);

	template <typename T>
	T* GetResource(std::string name);

	void LoadPipelineDictionary(const char* name, RenderPass* renderPass);

	void LoadPipelineByName(const char* name, RenderPass* renderPass);

private:
	RenderDevice* mDevice;

	std::map<std::string, ResourceTableLayout*> mGlobalDescriptorSetLayouts;

	std::map<std::type_index, std::map<std::string, ResourcePacketInterface*>> mNamedResourceInstances;

	std::vector<ResourcePacketInterface*> mResourceInstances;
};

template <typename T>
inline void ResourceManager::RegisterResource(std::string name, T* t, bool autoCleanup)
{
	std::type_index index = std::type_index(typeid(T));
	assert(mNamedResourceInstances[index].find(name) == mNamedResourceInstances[index].end());
	RegisterResource<T>(t, autoCleanup);
	mNamedResourceInstances[index][name] = new ResourceInstance<T>(t);
}

template <typename T>
inline void ResourceManager::RegisterResource(T* t, bool autoCleanup)
{
	for (uint32_t i = 0; i < mResourceInstances.size(); i++)
	{
		if (mResourceInstances[i]->GetPtr() == t)
			return;
	}
	mResourceInstances.push_back(new ResourceInstance<T>(t, autoCleanup));
}

template <typename T>
inline T* ResourceManager::GetResource(std::string name)
{
	std::type_index index = std::type_index(typeid(T));
	auto            it    = mNamedResourceInstances[index].find(name);
	assert(it != mNamedResourceInstances[index].end());

	return reinterpret_cast<T*>(it->second->GetPtr());
}

