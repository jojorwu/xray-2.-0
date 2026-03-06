#ifndef VulkanDescriptorManager_included
#define VulkanDescriptorManager_included
#pragma once

#include "VulkanHW.h"
#include "VulkanDescriptorSet.h"

struct DescriptorSetKey
{
    xr_vector<VkDescriptorImageInfo> images;
    xr_vector<VkDescriptorBufferInfo> buffers;

    bool operator<(const DescriptorSetKey& other) const
    {
        if (images.size() != other.images.size()) return images.size() < other.images.size();
        if (buffers.size() != other.buffers.size()) return buffers.size() < other.buffers.size();

        int res = memcmp(images.data(), other.images.data(), images.size() * sizeof(VkDescriptorImageInfo));
        if (res != 0) return res < 0;

        return memcmp(buffers.data(), other.buffers.data(), buffers.size() * sizeof(VkDescriptorBufferInfo)) < 0;
    }
};

class CVulkanDescriptorManager
{
public:
    CVulkanDescriptorManager();
    ~CVulkanDescriptorManager();

    void Create();
    void Destroy();

    VkDescriptorSet GetDescriptorSet(VkDescriptorSetLayout layout, const DescriptorSetKey& key);

private:
    CVulkanDescriptorPool m_pool;
    xr_map<DescriptorSetKey, VkDescriptorSet> m_cache;
};

extern CVulkanDescriptorManager VulkanDescriptorManager;

#endif // VulkanDescriptorManager_included
