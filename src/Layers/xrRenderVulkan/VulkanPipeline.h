#pragma once

#include "VulkanHW.h"

class CVulkanPipeline
{
public:
    CVulkanPipeline();
    ~CVulkanPipeline();

    void Create(VkPipelineLayout layout, VkRenderPass renderPass, const xr_vector<VkPipelineShaderStageCreateInfo>& shaderStages, const VkPipelineVertexInputStateCreateInfo& vertexInput, const VkPipelineRasterizationStateCreateInfo& rasterizer, const VkPipelineColorBlendStateCreateInfo& colorBlend, const VkPipelineDepthStencilStateCreateInfo& depthStencil, VkPrimitiveTopology topology);
    void Destroy();

    VkPipeline GetPipeline() { return m_pipeline; }
    VkPipelineLayout GetLayout() { return m_layout; }

private:
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;
};
