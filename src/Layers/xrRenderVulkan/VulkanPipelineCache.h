#ifndef VulkanPipelineCache_included
#define VulkanPipelineCache_included
#pragma once

#include "VulkanHW.h"

struct PipelineStateKey
{
    VkShaderModule vs;
    VkShaderModule ps;
    VkRenderPass renderPass;
    VkPipelineLayout layout;
    VkPrimitiveTopology topology;
    // States hash or key members
    uint32_t stateHash;

    bool operator<(const PipelineStateKey& other) const
    {
        return memcmp(this, &other, sizeof(PipelineStateKey)) < 0;
    }
};

class CVulkanPipelineCache
{
public:
    CVulkanPipelineCache();
    ~CVulkanPipelineCache();

    void Create();
    void Destroy();

    VkPipeline GetPipeline(const PipelineStateKey& key, const VkGraphicsPipelineCreateInfo& createInfo);

    void SaveCache();
    void LoadCache();

private:
    VkPipelineCache m_pipeline_cache;
    xr_map<PipelineStateKey, VkPipeline> m_pipelines;
};

extern CVulkanPipelineCache VulkanPipelineCache;

#endif // VulkanPipelineCache_included
