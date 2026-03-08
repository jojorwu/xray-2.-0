#ifndef VulkanDescriptorManager_included
#define VulkanDescriptorManager_included
#pragma once

#include "VulkanHW.h"
#include "VulkanDescriptorSet.h"

struct DescriptorSetKey
{
    xr_map<uint32_t, VkDescriptorImageInfo> images;
    xr_map<uint32_t, VkDescriptorBufferInfo> buffers;
    uint32_t hash;

    void ComputeHash();

    bool operator<(const DescriptorSetKey& other) const
    {
        if (hash != other.hash) return hash < other.hash;
        if (images != other.images) return images < other.images;

        if (buffers.size() != other.buffers.size()) return buffers.size() < other.buffers.size();
        auto it1 = buffers.begin();
        auto it2 = other.buffers.begin();
        while (it1 != buffers.end())
        {
            if (it1->first != it2->first) return it1->first < it2->first;
            if (it1->second.buffer != it2->second.buffer) return it1->second.buffer < it2->second.buffer;
            if (it1->second.range != it2->second.range) return it1->second.range < it2->second.range;
            // Skip offset for dynamic UBO
            ++it1; ++it2;
        }
        return false;
    }
};

inline bool operator<(const VkDescriptorImageInfo& a, const VkDescriptorImageInfo& b)
{
    if (a.sampler != b.sampler) return a.sampler < b.sampler;
    if (a.imageView != b.imageView) return a.imageView < b.imageView;
    return a.imageLayout < b.imageLayout;
}

inline bool operator<(const VkDescriptorBufferInfo& a, const VkDescriptorBufferInfo& b)
{
    if (a.buffer != b.buffer) return a.buffer < b.buffer;
    if (a.offset != b.offset) return a.offset < b.offset;
    return a.range < b.range;
}

class CVulkanDescriptorManager
{
public:
    CVulkanDescriptorManager();
    ~CVulkanDescriptorManager();

    void Create();
    void Destroy();

    VkDescriptorSet GetDescriptorSet(VkDescriptorSetLayout layout, const DescriptorSetKey& key);

    void UpdateBindless(uint32_t index, VkImageView view, VkSampler sampler);
    VkDescriptorSet GetBindlessSet() const { return m_bindless_set; }
    VkDescriptorSetLayout GetBindlessLayout() const { return m_bindless_layout; }

private:
    CVulkanDescriptorPool m_pool;
    xr_map<DescriptorSetKey, VkDescriptorSet> m_cache;

    VkDescriptorSetLayout m_bindless_layout;
    VkDescriptorPool m_bindless_pool;
    VkDescriptorSet m_bindless_set;
};

extern CVulkanDescriptorManager VulkanDescriptorManager;

#endif // VulkanDescriptorManager_included
