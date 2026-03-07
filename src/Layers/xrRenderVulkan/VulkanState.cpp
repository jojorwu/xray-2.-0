#include "stdafx.h"
#include "VulkanState.h"
#include "../xrRender/tss_def.h"

void CVulkanState::ConvertRasterizer(const SimulatorStates& states, VkPipelineRasterizationStateCreateInfo& info)
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_BACK_BIT;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    info.lineWidth = 1.0f;

    for (const auto& S : states.States)
    {
        if (S.type != 0) continue;
        switch (S.v1)
        {
        case D3DRS_CULLMODE:
            if (S.v2 == D3DCULL_NONE) info.cullMode = VK_CULL_MODE_NONE;
            else if (S.v2 == D3DCULL_CW) info.cullMode = VK_CULL_MODE_FRONT_BIT;
            else if (S.v2 == D3DCULL_CCW) info.cullMode = VK_CULL_MODE_BACK_BIT;
            break;
        case D3DRS_FILLMODE:
            if (S.v2 == D3DFILL_WIREFRAME) info.polygonMode = VK_POLYGON_MODE_LINE;
            else info.polygonMode = VK_POLYGON_MODE_FILL;
            break;
        }
    }
}

static VkBlendFactor ConvertBlendFactor(D3DBLEND factor)
{
    switch (factor)
    {
    case D3DBLEND_ZERO: return VK_BLEND_FACTOR_ZERO;
    case D3DBLEND_ONE: return VK_BLEND_FACTOR_ONE;
    case D3DBLEND_SRCCOLOR: return VK_BLEND_FACTOR_SRC_COLOR;
    case D3DBLEND_INVSRCCOLOR: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case D3DBLEND_SRCALPHA: return VK_BLEND_FACTOR_SRC_ALPHA;
    case D3DBLEND_INVSRCALPHA: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case D3DBLEND_DESTALPHA: return VK_BLEND_FACTOR_DST_ALPHA;
    case D3DBLEND_INVDESTALPHA: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case D3DBLEND_DESTCOLOR: return VK_BLEND_FACTOR_DST_COLOR;
    case D3DBLEND_INVDESTCOLOR: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    default: return VK_BLEND_FACTOR_ONE;
    }
}

static VkBlendOp ConvertBlendOp(D3DBLENDOP op)
{
    switch (op)
    {
    case D3DBLENDOP_ADD: return VK_BLEND_OP_ADD;
    case D3DBLENDOP_SUBTRACT: return VK_BLEND_OP_SUBTRACT;
    case D3DBLENDOP_REVSUBTRACT: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case D3DBLENDOP_MIN: return VK_BLEND_OP_MIN;
    case D3DBLENDOP_MAX: return VK_BLEND_OP_MAX;
    default: return VK_BLEND_OP_ADD;
    }
}

void CVulkanState::ConvertBlend(const SimulatorStates& states, VkPipelineColorBlendStateCreateInfo& info, xr_vector<VkPipelineColorBlendAttachmentState>& attachments)
{
    VkPipelineColorBlendAttachmentState att = {};
    att.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    att.blendEnable = VK_FALSE;
    att.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    att.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    att.colorBlendOp = VK_BLEND_OP_ADD;
    att.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    att.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    att.alphaBlendOp = VK_BLEND_OP_ADD;

    for (const auto& S : states.States)
    {
        if (S.type != 0) continue;
        switch (S.v1)
        {
        case D3DRS_ALPHABLENDENABLE: att.blendEnable = S.v2 ? VK_TRUE : VK_FALSE; break;
        case D3DRS_SRCBLEND: att.srcColorBlendFactor = ConvertBlendFactor((D3DBLEND)S.v2); break;
        case D3DRS_DESTBLEND: att.dstColorBlendFactor = ConvertBlendFactor((D3DBLEND)S.v2); break;
        case D3DRS_BLENDOP: att.colorBlendOp = ConvertBlendOp((D3DBLENDOP)S.v2); break;
        case D3DRS_SRCBLENDALPHA: att.srcAlphaBlendFactor = ConvertBlendFactor((D3DBLEND)S.v2); break;
        case D3DRS_DESTBLENDALPHA: att.dstAlphaBlendFactor = ConvertBlendFactor((D3DBLEND)S.v2); break;
        case D3DRS_BLENDOPALPHA: att.alphaBlendOp = ConvertBlendOp((D3DBLENDOP)S.v2); break;
        }
    }

    // X-Ray typically uses same blend for all MRTs or disables them
    for (u32 i = 0; i < 4; i++) attachments.push_back(att);

    info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    info.attachmentCount = (uint32_t)attachments.size();
    info.pAttachments = attachments.data();
}

static VkCompareOp ConvertCmpFunc(D3DCMPFUNC func)
{
    switch (func)
    {
    case D3DCMP_NEVER: return VK_COMPARE_OP_NEVER;
    case D3DCMP_LESS: return VK_COMPARE_OP_LESS;
    case D3DCMP_EQUAL: return VK_COMPARE_OP_EQUAL;
    case D3DCMP_LESSEQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
    case D3DCMP_GREATER: return VK_COMPARE_OP_GREATER;
    case D3DCMP_NOTEQUAL: return VK_COMPARE_OP_NOT_EQUAL;
    case D3DCMP_GREATEREQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case D3DCMP_ALWAYS: return VK_COMPARE_OP_ALWAYS;
    default: return VK_COMPARE_OP_LESS;
    }
}

void CVulkanState::ConvertDepthStencil(const SimulatorStates& states, VkPipelineDepthStencilStateCreateInfo& info)
{
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.depthTestEnable = VK_TRUE;
    info.depthWriteEnable = VK_TRUE;
    info.depthCompareOp = VK_COMPARE_OP_LESS;
    info.stencilTestEnable = VK_FALSE;

    for (const auto& S : states.States)
    {
        if (S.type != 0) continue;
        switch (S.v1)
        {
        case D3DRS_ZENABLE: info.depthTestEnable = S.v2 ? VK_TRUE : VK_FALSE; break;
        case D3DRS_ZWRITEENABLE: info.depthWriteEnable = S.v2 ? VK_TRUE : VK_FALSE; break;
        case D3DRS_ZFUNC: info.depthCompareOp = ConvertCmpFunc((D3DCMPFUNC)S.v2); break;
        case D3DRS_STENCILENABLE: info.stencilTestEnable = S.v2 ? VK_TRUE : VK_FALSE; break;
        }
    }
}
