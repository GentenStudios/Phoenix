#pragma once


class RenderDevice;
class RenderTarget;
class ResourceTableLayout;
class ResourceTable;
class PipelineLayout;
class Pipeline;
class Buffer;
class Texture;
class RenderTechnique;

class ResourcePacketInterface
{
public:
	ResourcePacketInterface( void* ptr ) : mPtr( ptr ) { }
	virtual ~ResourcePacketInterface( ) { };
	void* GetPtr( ) { return mPtr; }
protected:
	void* mPtr;
};

template <typename T>
class ResourceInstance : virtual public ResourcePacketInterface
{
public:
	ResourceInstance( T* t, bool autoCleanup = true) : ResourcePacketInterface( t ), mAutoCleanup(autoCleanup) { }
	T* Get( ) { return reinterpret_cast<T*>(mPtr); }
	virtual ~ResourceInstance( )
	{
		if(mAutoCleanup) delete reinterpret_cast<T*>(mPtr);
	}
private:
	bool mAutoCleanup;
};

template class ResourceInstance<RenderDevice>;
template class ResourceInstance<RenderTarget>;
template class ResourceInstance<ResourceTableLayout>;
template class ResourceInstance<ResourceTable>;
template class ResourceInstance<PipelineLayout>;
template class ResourceInstance<Pipeline>;
template class ResourceInstance<Buffer>;
template class ResourceInstance<Texture>;
template class ResourceInstance<RenderTechnique>;