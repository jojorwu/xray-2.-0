#pragma once

#include "VulkanHW.h"

class CVulkanBuffer
{
public:
    CVulkanBuffer();
    virtual ~CVulkanBuffer();

    void Create(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage mem_usage, VmaAllocationCreateFlags flags = 0);
    void Destroy();

    void Map(void** data, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
    void Unmap();
    void Flush(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
    void Invalidate(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

    VkBuffer GetBuffer() { return m_buffer; }
    VkDeviceSize GetSize() { return m_size; }
    VmaAllocation GetAllocation() { return m_allocation; }

    VkBuffer m_buffer;

protected:
    VmaAllocation m_allocation;
    VkDeviceSize m_size;
};

class CVulkanVertexBuffer : public CVulkanBuffer
{
public:
    void Create(uint32_t size, bool dynamic);
};

class CVulkanDynamicBuffer : public CVulkanBuffer
{
public:
    CVulkanDynamicBuffer();
    virtual ~CVulkanDynamicBuffer();

    void Create(VkDeviceSize size, VkBufferUsageFlags usage);
    void Destroy();
    uint32_t Alloc(uint32_t size, void** data);
    void FlushAlloc(uint32_t offset, uint32_t size);
    void Reset();

private:
    uint8_t* m_mapped_data;
    uint32_t m_current_offset;
};

class CVulkanUniformBuffer : public CVulkanBuffer
{
public:
    void Create(uint32_t size);
};

class CVulkanIndexBuffer : public CVulkanBuffer
{
public:
    void Create(uint32_t size, bool dynamic);
};
