#ifndef VulkanConstantBuffer_included
#define VulkanConstantBuffer_included
#pragma once

#include "../xrRender/SH_RT.h"

class CVulkanConstantBuffer : public xr_resource_named
{
public:
    CVulkanConstantBuffer(u32 size);
    ~CVulkanConstantBuffer();

    void* Data() { return m_data; }
    VkDescriptorBufferInfo VulkanUpdate();

private:
    void* m_data;
    u32 m_size;
};

typedef resptr_core<CVulkanConstantBuffer, resptr_base<CVulkanConstantBuffer>> ref_cbuffer;

#endif
