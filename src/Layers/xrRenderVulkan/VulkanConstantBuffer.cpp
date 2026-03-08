#include "stdafx.h"
#include "VulkanConstantBuffer.h"
#include "VulkanBackend.h"

CVulkanConstantBuffer::CVulkanConstantBuffer(u32 size)
{
    m_size = size;
    m_data = xr_malloc(size);
}

CVulkanConstantBuffer::~CVulkanConstantBuffer()
{
    xr_free(m_data);
}

VkDescriptorBufferInfo CVulkanConstantBuffer::VulkanUpdate()
{
    return VulkanBackend.AllocateUniform(m_size, m_data);
}
