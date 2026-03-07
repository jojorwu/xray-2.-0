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

void CVulkanShader::Create(const xr_vector<u32>& code)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(u32);
    createInfo.pCode = code.data();

    if (vkCreateShaderModule(VulkanHW.GetDevice(), &createInfo, nullptr, &m_module) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create shader module!");
    }
}

void CVulkanShader::Load(LPCSTR name)
{
    string_path path;
    strconcat(sizeof(path), path, "vulkan", FS.sep, name, ".spv");
    FS.update_path(path, "$game_shaders$", path);

    IReader* r = FS.r_open(path);
    if (!r)
    {
        Msg("! Vulkan: Shader %s not found!", path);
        return;
    }

    xr_vector<u32> code(r->length() / 4);
    r->r(code.data(), r->length());
    FS.r_close(r);

    Create(code);
}

void CVulkanShader::Destroy()
{
    if (m_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(VulkanHW.GetDevice(), m_module, nullptr);
        m_module = VK_NULL_HANDLE;
    }
}
