#pragma once

#include <vulkan.hpp>

#include <resourcepacket.hpp>
#include <memoryheap.hpp>

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <typeindex>
#include <assert.h>

class RenderDevice;
class DeviceMemory;
class ResourcePacketInterface;
class ResourceTableLayout;
class RenderTechnique;
class RenderPass;

class ResourceManager
{
public:
	ResourceManager( RenderDevice* device );
	~ResourceManager( );

	template <typename T>
	void RegisterResource( std::string name, T* t, bool autoCleanup = true);

	template <typename T>
	void RegisterResource( T* t, bool autoCleanup = true );

	template <typename T>
	T* GetResource( std::string name );

	void LoadPipelineDictionary( const char* name, RenderPass* renderPass );

	void LoadPipelineByName( const char* name, RenderPass* renderPass );

private:
	RenderDevice* mDevice;

	std::map<std::string, ResourceTableLayout*> mGlobalDescriptorSetLayouts;

	std::map<std::type_index, std::map<std::string, ResourcePacketInterface*>> mNamedResourceInstances;

	std::vector<ResourcePacketInterface*> mResourceInstances;
};

template<typename T>
inline void ResourceManager::RegisterResource( std::string name, T* t, bool autoCleanup)
{
	std::type_index index = std::type_index( typeid(T) );
	assert( mNamedResourceInstances[index].find( name ) == mNamedResourceInstances[index].end( ) );
	RegisterResource<T>( t, autoCleanup);
	mNamedResourceInstances[index][name] = new ResourceInstance<T>( t );
}

template<typename T>
inline void ResourceManager::RegisterResource( T* t, bool autoCleanup)
{
	for ( uint32_t i = 0; i < mResourceInstances.size( ); i++ )
	{
		if ( mResourceInstances[i]->GetPtr( ) == t ) return;
	}
	mResourceInstances.push_back( new ResourceInstance<T>( t, autoCleanup) );
}

template<typename T>
inline T* ResourceManager::GetResource( std::string name )
{
	std::type_index index = std::type_index( typeid(T) );
	auto it = mNamedResourceInstances[index].find( name );
	assert( it != mNamedResourceInstances[index].end( ) );

	return reinterpret_cast<T*>(it->second->GetPtr());
}
