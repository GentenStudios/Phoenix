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

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>

#include <Globals/Globals.hpp>

#include <Phoenix/Statistics.hpp>

class Window;
class RenderDevice;
class ResourceManager;
class RenderTarget;
class MemoryHeap;
class Camera;
class RenderTechnique;
class Buffer;
class DebugUI;

namespace phx
{
	class World;
	class InputHandler;
	class ModHandler;

	class Phoenix
	{
	public:
		Phoenix(Window* window);

		~Phoenix();

		void RebuildCommandBuffers();

		void Update();

		void Validate();

		RenderTarget* GetPrimaryRenderTarget() { return mPrimaryRenderTarget; }

		MemoryHeap* GetDeviceLocalMemoryHeap() { return mDeviceLocalMemoryHeap.get(); }

		MemoryHeap* GetGPUMappableMemoryHeap() { return mGPUMappableMemoryHeap.get(); }

		StatisticManager& GetStatistics() { return mStatisticManager; }

		ResourceManager* GetResourceManager() { return mResourceManager.get(); }

		Window* GetWindow();

	private:
		void UpdateCamera();

		void RebuildRenderPassResources();

		void CreateRenderPassResource();

		void CreateMemoryHeaps();

		void DestroyMemoryHeaps();

		void CreateCameraBuffer();

		void InitCamera();

		void InitWorld();

		void InitMods();

		void InitDebugUI();

		void InitInputHandler();

		void InitTexturePool();

		void InitDefaultTextures();

		Window* mWindow;
		
		std::unique_ptr<RenderDevice>    mDevice;
		std::unique_ptr<ResourceManager> mResourceManager;

		std::unique_ptr<World> mWorld;
		std::unique_ptr<ModHandler> mMods;

		std::unique_ptr<DebugUI> mDebugUI;
		std::unique_ptr<InputHandler> mInputHandler;
		
		std::unique_ptr<MemoryHeap> mDeviceLocalMemoryHeap;
		std::unique_ptr<MemoryHeap> mGPUMappableMemoryHeap;

		RenderTarget* mPrimaryRenderTarget = nullptr;

		StatisticManager mStatisticManager;

		float mDeltaTime;

		Camera* mCamera;
		Buffer* mCameraBuffer;

		static Phoenix* mInstance;
	};
} // namespace phx

