#include "stdafx.h"
#include "VulkanDescriptorSet.h"

CVulkanDescriptorSet::CVulkanDescriptorSet()
{
    m_descriptor_set = VK_NULL_HANDLE;
}

CVulkanDescriptorSet::~CVulkanDescriptorSet()
{
}

void CVulkanDescriptorSet::Create(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(VulkanHW.GetDevice(), &allocInfo, &m_descriptor_set) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to allocate descriptor set!");
    }
}

void CVulkanDescriptorSet::Update(const xr_vector<VkWriteDescriptorSet>& writes)
{
    vkUpdateDescriptorSets(VulkanHW.GetDevice(), (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

CVulkanDescriptorPool::CVulkanDescriptorPool()
{
    m_pool = VK_NULL_HANDLE;
}

CVulkanDescriptorPool::~CVulkanDescriptorPool()
{
    Destroy();
}

void CVulkanDescriptorPool::Create(const xr_vector<VkDescriptorPoolSize>& sizes, uint32_t maxSets)
{
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = (uint32_t)sizes.size();
    poolInfo.pPoolSizes = sizes.data();
    poolInfo.maxSets = maxSets;

    if (vkCreateDescriptorPool(VulkanHW.GetDevice(), &poolInfo, nullptr, &m_pool) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create descriptor pool!");
    }
}

void CVulkanDescriptorPool::Destroy()
{
    if (m_pool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(VulkanHW.GetDevice(), m_pool, nullptr);
        m_pool = VK_NULL_HANDLE;
    }
}
