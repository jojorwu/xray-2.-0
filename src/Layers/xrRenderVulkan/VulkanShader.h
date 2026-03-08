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
    VkShaderModule ExtractModule() { VkShaderModule m = m_module; m_module = VK_NULL_HANDLE; return m; }

    struct ReflectionInfo
    {
        xr_vector<std::pair<u32, u32>> constant_buffers; // binding, size
    };
    const ReflectionInfo& GetReflection() const { return m_reflection; }

private:
    VkShaderModule m_module;
    ReflectionInfo m_reflection;
};
