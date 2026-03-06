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

VkPipeline CVulkanPipelineCache::GetPipeline(const PipelineStateKey& key, const VkGraphicsPipelineCreateInfo& createInfo)
{
    auto it = m_pipelines.find(key);
    if (it != m_pipelines.end())
        return it->second;

    VkPipeline pipeline;
    VkGraphicsPipelineCreateInfo info = createInfo;
    info.basePipelineHandle = VK_NULL_HANDLE;
    info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(VulkanHW.GetDevice(), m_pipeline_cache, 1, &info, nullptr, &pipeline) != VK_SUCCESS)
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
