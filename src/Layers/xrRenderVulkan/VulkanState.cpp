#include "stdafx.h"
#include "VulkanState.h"

void CVulkanState::ConvertRasterizer(const SimulatorStates& states, VkPipelineRasterizationStateCreateInfo& info)
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_BACK_BIT;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    info.lineWidth = 1.0f;
}

void CVulkanState::ConvertBlend(const SimulatorStates& states, VkPipelineColorBlendStateCreateInfo& info, xr_vector<VkPipelineColorBlendAttachmentState>& attachments)
{
    VkPipelineColorBlendAttachmentState attachment = {};
    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    attachment.blendEnable = VK_FALSE;
    attachments.push_back(attachment);

    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.attachmentCount = (uint32_t)attachments.size();
    info.pAttachments = attachments.data();
}

void CVulkanState::ConvertDepthStencil(const SimulatorStates& states, VkPipelineDepthStencilStateCreateInfo& info)
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.depthTestEnable = VK_TRUE;
    info.depthWriteEnable = VK_TRUE;
    info.depthCompareOp = VK_COMPARE_OP_LESS;
}
