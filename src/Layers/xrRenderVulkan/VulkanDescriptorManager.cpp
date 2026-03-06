#include "stdafx.h"
#include "VulkanDescriptorManager.h"

CVulkanDescriptorManager VulkanDescriptorManager;

CVulkanDescriptorManager::CVulkanDescriptorManager()
{
}

CVulkanDescriptorManager::~CVulkanDescriptorManager()
{
    Destroy();
}

void CVulkanDescriptorManager::Create()
{
    xr_vector<VkDescriptorPoolSize> sizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 }
    };
    m_pool.Create(sizes, 1024);
}

void CVulkanDescriptorManager::Destroy()
{
    m_cache.clear();
    m_pool.Destroy();
}

VkDescriptorSet CVulkanDescriptorManager::GetDescriptorSet(VkDescriptorSetLayout layout, const DescriptorSetKey& key)
{
    auto it = m_cache.find(key);
    if (it != m_cache.end())
        return it->second;

    CVulkanDescriptorSet set;
    set.Create(m_pool.GetPool(), layout);

    xr_vector<VkWriteDescriptorSet> writes;
    for (uint32_t i = 0; i < key.buffers.size(); i++)
    {
        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set.GetSet();
        write.dstBinding = i;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &key.buffers[i];
        writes.push_back(write);
    }

    uint32_t bindingOffset = (uint32_t)key.buffers.size();
    for (uint32_t i = 0; i < key.images.size(); i++)
    {
        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set.GetSet();
        write.dstBinding = bindingOffset + i;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &key.images[i];
        writes.push_back(write);
    }

    set.Update(writes);

    m_cache[key] = set.GetSet();
    return set.GetSet();
}
