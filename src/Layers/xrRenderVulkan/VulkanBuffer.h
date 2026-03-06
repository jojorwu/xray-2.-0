#pragma once

#include "VulkanHW.h"

class CVulkanBuffer
{
public:
    CVulkanBuffer();
    virtual ~CVulkanBuffer();

    void Create(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage mem_usage, VmaAllocationCreateFlags flags = 0);
    void Destroy();

    void Map(void** data);
    void Unmap();
    void Flush();
    void Invalidate();

    VkBuffer GetBuffer() { return m_buffer; }
    VkDeviceSize GetSize() { return m_size; }
    VmaAllocation GetAllocation() { return m_allocation; }

    void AddRef() {}
    void Release() { /* Managed by engine */ }

protected:
    VkBuffer m_buffer;
    VmaAllocation m_allocation;
    VkDeviceSize m_size;
};

class CVulkanVertexBuffer : public CVulkanBuffer
{
public:
    void Create(uint32_t size, bool dynamic);
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
