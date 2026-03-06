#pragma once

#include "VulkanHW.h"

class CVulkanShader
{
public:
    CVulkanShader();
    ~CVulkanShader();

    void Create(const xr_vector<uint32_t>& code);
    void Destroy();

    VkShaderModule GetModule() { return m_module; }

private:
    VkShaderModule m_module;
};
