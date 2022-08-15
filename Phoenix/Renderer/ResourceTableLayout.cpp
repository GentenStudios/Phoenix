#include <Renderer/Device.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>

#include <cassert>
#include <cstring>

ResourceTableLayout::ResourceTableLayout(RenderDevice* device, VkDescriptorSetLayoutBinding* descriptorSetLayoutBinding,
                                         uint32_t descriptorSetLayoutBindingCount, uint32_t maxSets)
    : m_device(device)
{
	m_descriptorSetLayoutBindings =
	    std::unique_ptr<VkDescriptorSetLayoutBinding>(new VkDescriptorSetLayoutBinding[descriptorSetLayoutBindingCount]);

	std::memcpy(m_descriptorSetLayoutBindings.get(), descriptorSetLayoutBinding,
	            sizeof(VkDescriptorSetLayoutBinding) * descriptorSetLayoutBindingCount);

	m_descriptorSetLayoutBindingCount = descriptorSetLayoutBindingCount;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount                    = descriptorSetLayoutBindingCount;
	descriptorSetLayoutCreateInfo.pBindings                       = descriptorSetLayoutBinding;

	m_device->Validate(vkCreateDescriptorSetLayout(m_device->GetDevice(), &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorSetLayout));

	auto descriptorPoolSizes = std::make_unique<VkDescriptorPoolSize[]>(descriptorSetLayoutBindingCount);

	for (uint32_t i = 0; i < descriptorSetLayoutBindingCount; ++i)
	{
		descriptorPoolSizes[i].type            = descriptorSetLayoutBinding[i].descriptorType;
		descriptorPoolSizes[i].descriptorCount = maxSets;
	}

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.poolSizeCount              = descriptorSetLayoutBindingCount;
	descriptorPoolCreateInfo.pPoolSizes                 = descriptorPoolSizes.get();
	descriptorPoolCreateInfo.maxSets                    = maxSets;

	m_device->Validate(vkCreateDescriptorPool(m_device->GetDevice(), &descriptorPoolCreateInfo, nullptr, &m_descriptorPool));
}

ResourceTableLayout::~ResourceTableLayout()
{
	vkDestroyDescriptorSetLayout(m_device->GetDevice(), m_descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(m_device->GetDevice(), m_descriptorPool, nullptr);
}

ResourceTable* ResourceTableLayout::CreateTable()
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool              = m_descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount          = 1;
	descriptorSetAllocateInfo.pSetLayouts                 = &m_descriptorSetLayout;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	m_device->Validate(vkAllocateDescriptorSets(m_device->GetDevice(), &descriptorSetAllocateInfo, &descriptorSet));

	return new ResourceTable(m_device, this, descriptorSet);
}

VkDescriptorPool ResourceTableLayout::GetDescriptorPool() const { return m_descriptorPool; }

VkDescriptorSetLayout ResourceTableLayout::GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

VkDescriptorType ResourceTableLayout::GetDescriptorType(uint32_t index) const
{
	assert(index < m_descriptorSetLayoutBindingCount);
	return m_descriptorSetLayoutBindings.get()[index].descriptorType;
}
