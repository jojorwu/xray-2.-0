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

void CVulkanBuffer::Map(void** data, VkDeviceSize offset, VkDeviceSize size)
{
    vmaMapMemory(VulkanHW.GetAllocator(), m_allocation, data);
}

void CVulkanBuffer::Unmap()
{
    vmaUnmapMemory(VulkanHW.GetAllocator(), m_allocation);
}

void CVulkanBuffer::Flush(VkDeviceSize offset, VkDeviceSize size)
{
    vmaFlushAllocation(VulkanHW.GetAllocator(), m_allocation, offset, size);
}

void CVulkanBuffer::Invalidate(VkDeviceSize offset, VkDeviceSize size)
{
    vmaInvalidateAllocation(VulkanHW.GetAllocator(), m_allocation, offset, size);
}

void CVulkanVertexBuffer::Create(uint32_t size, bool dynamic)
{
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VmaMemoryUsage mem_usage = dynamic ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;
    VmaAllocationCreateFlags flags = dynamic ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

    CVulkanBuffer::Create(size, usage, mem_usage, flags);
}

CVulkanDynamicBuffer::CVulkanDynamicBuffer()
{
    m_mapped_data = nullptr;
    m_current_offset = 0;
}

CVulkanDynamicBuffer::~CVulkanDynamicBuffer()
{
    if (m_mapped_data) vmaUnmapMemory(VulkanHW.GetAllocator(), m_allocation);
}

void CVulkanDynamicBuffer::Create(VkDeviceSize size, VkBufferUsageFlags usage)
{
    CVulkanBuffer::Create(size, usage, VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT);
    vmaMapMemory(VulkanHW.GetAllocator(), m_allocation, (void**)&m_mapped_data);
}

void CVulkanDynamicBuffer::Destroy()
{
    if (m_mapped_data) vmaUnmapMemory(VulkanHW.GetAllocator(), m_allocation);
    m_mapped_data = nullptr;
    CVulkanBuffer::Destroy();
}

uint32_t CVulkanDynamicBuffer::Alloc(uint32_t size, void** data)
{
    // Alignment
    uint32_t alignment = 256; // Standard for UBO, good for others too
    m_current_offset = (m_current_offset + alignment - 1) & ~(alignment - 1);

    if (m_current_offset + size > m_size)
    {
        Msg("! Vulkan: Dynamic buffer overflow!");
        return 0xFFFFFFFF;
    }

    *data = m_mapped_data + m_current_offset;
    uint32_t res = m_current_offset;
    m_current_offset += size;
    return res;
}

void CVulkanDynamicBuffer::FlushAlloc(uint32_t offset, uint32_t size)
{
    Flush(offset, size);
}

void CVulkanDynamicBuffer::Reset()
{
    m_current_offset = 0;
}

void CVulkanUniformBuffer::Create(uint32_t size)
{
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VmaMemoryUsage mem_usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    CVulkanBuffer::Create(size, usage, mem_usage, flags);
}

void CVulkanIndexBuffer::Create(uint32_t size, bool dynamic)
{
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VmaMemoryUsage mem_usage = dynamic ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;
    VmaAllocationCreateFlags flags = dynamic ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

    CVulkanBuffer::Create(size, usage, mem_usage, flags);
}
