#pragma once

#include "VulkanHW.h"

class CVulkanShader
{
public:
    CVulkanShader();
    ~CVulkanShader();

    void Create(const xr_vector<u32>& code);
    void Load(LPCSTR name);
    void Destroy();

    VkShaderModule GetModule() { return m_module; }

private:
    VkShaderModule m_module;
};
