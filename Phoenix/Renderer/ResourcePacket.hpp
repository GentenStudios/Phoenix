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
	explicit ResourcePacketInterface(void* ptr) : m_ptr(ptr) {}
	virtual ~ResourcePacketInterface() = default;
	void*    GetPtr() const { return m_ptr; }

protected:
	void* m_ptr;
};

// Disable warnings bc this works but compiler no like.
#pragma warning(push, 0)
template <typename T>
class ResourceInstance : virtual public ResourcePacketInterface
{
public:
	explicit ResourceInstance(T* t, bool autoCleanup = true) : ResourcePacketInterface(t), m_autoCleanup(autoCleanup) {}

	T* Get() { return static_cast<T*>(m_ptr); }

	~ResourceInstance() override
	{
		if (m_autoCleanup)
			delete static_cast<T*>(m_ptr);
	}

private:
	bool m_autoCleanup;
};
#pragma warning(pop)

template class ResourceInstance<RenderDevice>;
template class ResourceInstance<RenderTarget>;
template class ResourceInstance<ResourceTableLayout>;
template class ResourceInstance<ResourceTable>;
template class ResourceInstance<PipelineLayout>;
template class ResourceInstance<Pipeline>;
template class ResourceInstance<Buffer>;
template class ResourceInstance<Texture>;
template class ResourceInstance<RenderTechnique>;