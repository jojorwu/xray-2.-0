#include "stdafx.h"
#include "VulkanDescriptorManager.h"

CVulkanDescriptorManager VulkanDescriptorManager;

void DescriptorSetKey::ComputeHash()
{
    hash = 0;
    for (auto const& [binding, info] : buffers)
    {
        hash = crc32(&binding, sizeof(binding), hash);
        hash = crc32(&info.buffer, sizeof(info.buffer), hash);
        hash = crc32(&info.offset, sizeof(info.offset), hash);
        hash = crc32(&info.range, sizeof(info.range), hash);
    }
    for (auto const& [binding, info] : images)
    {
        hash = crc32(&binding, sizeof(binding), hash);
        hash = crc32(&info.sampler, sizeof(info.sampler), hash);
        hash = crc32(&info.imageView, sizeof(info.imageView), hash);
    }
}

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
    for (auto const& [binding, info] : key.buffers)
    {
        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set.GetSet();
        write.dstBinding = binding;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &info;
        writes.push_back(write);
    }

    for (auto const& [binding, info] : key.images)
    {
        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set.GetSet();
        write.dstBinding = 4 + binding; // Textures start at binding 4
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &info;
        writes.push_back(write);
    }

    set.Update(writes);

    m_cache[key] = set.GetSet();
    return set.GetSet();
}
