#include "stdafx.h"
#include "VulkanPipelineCache.h"

CVulkanPipelineCache VulkanPipelineCache;

CVulkanPipelineCache::CVulkanPipelineCache()
{
    m_pipeline_cache = VK_NULL_HANDLE;
}

CVulkanPipelineCache::~CVulkanPipelineCache()
{
    Destroy();
}

void CVulkanPipelineCache::Create()
{
    // Load data from file if exists
    LoadCache();

    if (m_pipeline_cache == VK_NULL_HANDLE)
    {
        VkPipelineCacheCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        if (vkCreatePipelineCache(VulkanHW.GetDevice(), &createInfo, nullptr, &m_pipeline_cache) != VK_SUCCESS)
        {
            Msg("! Vulkan: Failed to create pipeline cache!");
        }
    }
}

void CVulkanPipelineCache::Destroy()
{
    SaveCache();

    for (auto& pair : m_pipelines)
    {
        vkDestroyPipeline(VulkanHW.GetDevice(), pair.second, nullptr);
    }
    m_pipelines.clear();

    if (m_pipeline_cache != VK_NULL_HANDLE)
    {
        vkDestroyPipelineCache(VulkanHW.GetDevice(), m_pipeline_cache, nullptr);
        m_pipeline_cache = VK_NULL_HANDLE;
    }
}

VkPipeline CVulkanPipelineCache::GetPipeline(const PipelineStateKey& key)
{
    auto it = m_pipelines.find(key);
    if (it != m_pipelines.end())
        return it->second;

    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = key.vs;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = key.ps;
    shaderStages[1].pName = "main";

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = key.topology;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = key.colorAttachmentCount;
    colorBlending.pAttachments = key.blendAttachments.data();

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    xr_vector<VkVertexInputBindingDescription> bindings;
    xr_vector<VkVertexInputAttributeDescription> attributes;
    SDeclaration* dcl = (SDeclaration*)(intptr_t)key.inputLayoutHash;
    if (dcl)
    {
        CVulkanInputLayout::Convert(dcl->dcl_code.data(), bindings, attributes);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)bindings.size();
    vertexInputInfo.pVertexBindingDescriptions = bindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &key.rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &key.depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = key.layout;
    pipelineInfo.renderPass = key.renderPass;
    pipelineInfo.subpass = 0;

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(VulkanHW.GetDevice(), m_pipeline_cache, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create graphics pipeline in cache!");
        return VK_NULL_HANDLE;
    }

    m_pipelines[key] = pipeline;
    return pipeline;
}

void CVulkanPipelineCache::SaveCache()
{
    if (m_pipeline_cache == VK_NULL_HANDLE) return;

    size_t size = 0;
    vkGetPipelineCacheData(VulkanHW.GetDevice(), m_pipeline_cache, &size, nullptr);

    if (size > 0)
    {
        xr_vector<uint8_t> data(size);
        vkGetPipelineCacheData(VulkanHW.GetDevice(), m_pipeline_cache, &size, data.data());

        string_path path;
        FS.update_path(path, "$game_data$", "vulkan_pipeline_cache.bin");
        IWriter* w = FS.w_open(path);
        if (w)
        {
            w->w(data.data(), (uint32_t)size);
            FS.w_close(w);
            Msg("Vulkan: Pipeline cache saved to %s", path);
        }
    }
}

void CVulkanPipelineCache::LoadCache()
{
    string_path path;
    FS.update_path(path, "$game_data$", "vulkan_pipeline_cache.bin");
    IReader* r = FS.r_open(path);
    if (r)
    {
        size_t size = r->length();
        xr_vector<uint8_t> data(size);
        r->r(data.data(), (uint32_t)size);
        FS.r_close(r);

        VkPipelineCacheCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.initialDataSize = size;
        createInfo.pInitialData = data.data();

        if (vkCreatePipelineCache(VulkanHW.GetDevice(), &createInfo, nullptr, &m_pipeline_cache) != VK_SUCCESS)
        {
            Msg("! Vulkan: Failed to create pipeline cache with initial data!");
        }
        else
        {
            Msg("Vulkan: Pipeline cache loaded from %s (%zu bytes)", path, size);
        }
    }
}
