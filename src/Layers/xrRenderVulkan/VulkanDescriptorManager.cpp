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
        // hash = crc32(&info.offset, sizeof(info.offset), hash); // Skip offset for dynamic UBO
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

    // Bindless setup
    uint32_t max_textures = 16384;
    VkDescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = max_textures;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorBindingFlags binding_flags =
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_info = {};
    flags_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flags_info.bindingCount = 1;
    flags_info.pBindingFlags = &binding_flags;

    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.pNext = &flags_info;
    layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &binding;

    vkCreateDescriptorSetLayout(VulkanHW.GetDevice(), &layout_info, nullptr, &m_bindless_layout);

    VkDescriptorPoolSize pool_size = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, max_textures };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;

    vkCreateDescriptorPool(VulkanHW.GetDevice(), &pool_info, nullptr, &m_bindless_pool);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = m_bindless_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &m_bindless_layout;

    vkAllocateDescriptorSets(VulkanHW.GetDevice(), &alloc_info, &m_bindless_set);
}

void CVulkanDescriptorManager::Destroy()
{
    m_cache.clear();
    m_pool.Destroy();
    if (m_bindless_set != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(VulkanHW.GetDevice(), m_bindless_pool, nullptr);
        vkDestroyDescriptorSetLayout(VulkanHW.GetDevice(), m_bindless_layout, nullptr);
    }
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
        VkDescriptorBufferInfo dynamic_info = info;
        dynamic_info.offset = 0; // Use base offset for dynamic descriptors

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = set.GetSet();
        write.dstBinding = binding;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        write.descriptorCount = 1;
        write.pBufferInfo = &dynamic_info;
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

void CVulkanDescriptorManager::UpdateBindless(uint32_t index, VkImageView view, VkSampler sampler)
{
    VkDescriptorImageInfo image_info = {};
    image_info.sampler = sampler;
    image_info.imageView = view;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_bindless_set;
    write.dstBinding = 0;
    write.dstArrayElement = index;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &image_info;

    vkUpdateDescriptorSets(VulkanHW.GetDevice(), 1, &write, 0, nullptr);
}
