
#include <Renderer/framebufferattachment.hpp>
#include <Renderer/device.hpp>
#include <Renderer/texture.hpp>
#include <Renderer/devicememory.hpp>

FramebufferPacket::FramebufferPacket( Texture* textures, uint32_t textureCount, VkFormat format, VkImageLayout layout, EFramebufferImageType imageType ) :
	mImageViewCount( textureCount ), mFormat( format ), mLayout( layout ), mImageType( imageType )
{
	mImageViews = std::unique_ptr <VkImageView>( new VkImageView[textureCount] );
	for ( uint32_t i = 0 ; i < textureCount ;i++)
	{
		VkImageView im = textures[i].GetImageView( );
		mImageViews.get( )[i] = im;
	}
}

FramebufferPacket::FramebufferPacket( VkImageView* imageViews, uint32_t textureCount, VkFormat format, VkImageLayout layout, EFramebufferImageType imageType ) :
	mImageViewCount( textureCount ), mFormat( format ), mLayout( layout ), mImageType( imageType )
{
	mImageViews = std::unique_ptr <VkImageView>( new VkImageView[textureCount] );
	for ( uint32_t i = 0; i < textureCount; i++ )
	{
		VkImageView im = imageViews[i];
		mImageViews.get( )[i] = im;
	}
}

FramebufferPacket::~FramebufferPacket( )
{
}

FramebufferAttachment::FramebufferAttachment( RenderDevice* device, std::vector< FramebufferPacket*> framebufferPackets ) :
	mDevice( device ), mFramebufferAttachmentCount( static_cast<uint32_t>(framebufferPackets.size( )) ), mFramebufferPackets( framebufferPackets )
{
	mFramebufferCount = mDevice->GetSwapchainImageCount( );
	

	mFramebufferAttachments = new VkImageView*[mFramebufferCount];

	for ( uint32_t i = 0; i < mFramebufferCount; i++ )
	{
		mFramebufferAttachments[i] = new VkImageView[mFramebufferAttachmentCount];
		for ( uint32_t j = 0; j < mFramebufferAttachmentCount; j++ )
		{
			mFramebufferAttachments[i][j] = framebufferPackets[j]->GetImageViews( )[i % framebufferPackets[j]->GetImageViewCount( )];
		}
	}
}

FramebufferAttachment::~FramebufferAttachment( )
{
	for ( uint32_t i = 0; i < mFramebufferCount; i++ )
	{
		delete[] mFramebufferAttachments[i];
	}
	delete[] mFramebufferAttachments;
}