#pragma once

#include "VulkanHW.h"

class CVulkanDescriptorSet
{
public:
    CVulkanDescriptorSet();
    ~CVulkanDescriptorSet();

    void Create(VkDescriptorPool pool, VkDescriptorSetLayout layout);
    void Update(const xr_vector<VkWriteDescriptorSet>& writes);

    VkDescriptorSet GetSet() { return m_descriptor_set; }

private:
    VkDescriptorSet m_descriptor_set;
};

class CVulkanDescriptorPool
{
public:
    CVulkanDescriptorPool();
    ~CVulkanDescriptorPool();

    void Create(const xr_vector<VkDescriptorPoolSize>& sizes, uint32_t maxSets);
    void Destroy();

    VkDescriptorPool GetPool() { return m_pool; }

private:
    VkDescriptorPool m_pool;
};
