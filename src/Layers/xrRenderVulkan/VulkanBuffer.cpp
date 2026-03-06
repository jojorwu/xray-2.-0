#include "stdafx.h"
#include "VulkanBuffer.h"

CVulkanBuffer::CVulkanBuffer()
{
    m_buffer = VK_NULL_HANDLE;
    m_allocation = VK_NULL_HANDLE;
    m_size = 0;
}

CVulkanBuffer::~CVulkanBuffer()
{
    Destroy();
}

void CVulkanBuffer::Create(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage mem_usage, VmaAllocationCreateFlags flags)
{
    m_size = size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = mem_usage;
    allocInfo.flags = flags;

    if (vmaCreateBuffer(VulkanHW.GetAllocator(), &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create buffer!");
    }
}

void CVulkanBuffer::Destroy()
{
    if (m_buffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(VulkanHW.GetAllocator(), m_buffer, m_allocation);
        m_buffer = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
    }
}

void CVulkanBuffer::Map(void** data)
{
    vmaMapMemory(VulkanHW.GetAllocator(), m_allocation, data);
}

void CVulkanBuffer::Unmap()
{
    vmaUnmapMemory(VulkanHW.GetAllocator(), m_allocation);
}

void CVulkanBuffer::Flush()
{
    vmaFlushAllocation(VulkanHW.GetAllocator(), m_allocation, 0, VK_WHOLE_SIZE);
}

void CVulkanBuffer::Invalidate()
{
    vmaInvalidateAllocation(VulkanHW.GetAllocator(), m_allocation, 0, VK_WHOLE_SIZE);
}

void CVulkanVertexBuffer::Create(uint32_t size, bool dynamic)
{
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VmaMemoryUsage mem_usage = dynamic ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;
    VmaAllocationCreateFlags flags = dynamic ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

    CVulkanBuffer::Create(size, usage, mem_usage, flags);
}

void CVulkanIndexBuffer::Create(uint32_t size, bool dynamic)
{
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VmaMemoryUsage mem_usage = dynamic ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;
    VmaAllocationCreateFlags flags = dynamic ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

    CVulkanBuffer::Create(size, usage, mem_usage, flags);
}
