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
    uint32_t inputLayoutHash;

    // Fixed function states
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    VkStencilOpState front;
    VkStencilOpState back;
    xr_array<VkPipelineColorBlendAttachmentState, 4> blendAttachments;
    uint32_t colorAttachmentCount;

    bool operator<(const PipelineStateKey& other) const
    {
        if (vs != other.vs) return vs < other.vs;
        if (ps != other.ps) return ps < other.ps;
        if (renderPass != other.renderPass) return renderPass < other.renderPass;
        if (layout != other.layout) return layout < other.layout;
        if (topology != other.topology) return topology < other.topology;
        if (inputLayoutHash != other.inputLayoutHash) return inputLayoutHash < other.inputLayoutHash;
        if (colorAttachmentCount != other.colorAttachmentCount) return colorAttachmentCount < other.colorAttachmentCount;

        int res;
        res = memcmp(&rasterizer, &other.rasterizer, sizeof(rasterizer));
        if (res != 0) return res < 0;
        res = memcmp(&depthStencil, &other.depthStencil, sizeof(depthStencil));
        if (res != 0) return res < 0;
        res = memcmp(&front, &other.front, sizeof(front));
        if (res != 0) return res < 0;
        res = memcmp(&back, &other.back, sizeof(back));
        if (res != 0) return res < 0;

        for (u32 i = 0; i < colorAttachmentCount; i++)
        {
            res = memcmp(&blendAttachments[i], &other.blendAttachments[i], sizeof(VkPipelineColorBlendAttachmentState));
            if (res != 0) return res < 0;
        }
        return false;
    }
};

class CVulkanPipelineCache
{
public:
    CVulkanPipelineCache();
    ~CVulkanPipelineCache();

    void Create();
    void Destroy();

    VkPipeline GetPipeline(const PipelineStateKey& key);

    void SaveCache();
    void LoadCache();

private:
    VkPipelineCache m_pipeline_cache;
    xr_map<PipelineStateKey, VkPipeline> m_pipelines;
};

extern CVulkanPipelineCache VulkanPipelineCache;

#endif // VulkanPipelineCache_included
