#pragma once

#include "VulkanHW.h"

class CVulkanState
{
public:
    static void ConvertRasterizer(const SimulatorStates& states, VkPipelineRasterizationStateCreateInfo& info);
    static void ConvertBlend(const SimulatorStates& states, VkPipelineColorBlendStateCreateInfo& info, xr_vector<VkPipelineColorBlendAttachmentState>& attachments);
    static void ConvertDepthStencil(const SimulatorStates& states, VkPipelineDepthStencilStateCreateInfo& info);
};
