#include "stdafx.h"
#include "VulkanShader.h"

CVulkanShader::CVulkanShader()
{
    m_module = VK_NULL_HANDLE;
}

CVulkanShader::~CVulkanShader()
{
    Destroy();
}

void CVulkanShader::Create(const xr_vector<uint32_t>& code)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    if (vkCreateShaderModule(VulkanHW.GetDevice(), &createInfo, nullptr, &m_module) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create shader module!");
    }
}

void CVulkanShader::Destroy()
{
    if (m_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(VulkanHW.GetDevice(), m_module, nullptr);
        m_module = VK_NULL_HANDLE;
    }
}
