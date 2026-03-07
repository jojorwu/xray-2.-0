#include "stdafx.h"
#include "VulkanVertexInputState.h"
#include "VulkanInputLayout.h"

CVulkanVertexInputCache VulkanVertexInputCache;

CVulkanVertexInputCache::CVulkanVertexInputCache()
{
}

CVulkanVertexInputCache::~CVulkanVertexInputCache()
{
}

VkPipelineVertexInputStateCreateInfo* CVulkanVertexInputCache::GetState(D3DVERTEXELEMENT9* dcl)
{
    uint32_t key = (uint32_t)(intptr_t)dcl;
    auto it = m_cache.find(key);
    if (it != m_cache.end())
        return &it->second.info;

    CachedState state;
    CVulkanInputLayout::Convert(dcl, state.bindings, state.attributes);

    state.info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    state.info.vertexBindingDescriptionCount = (uint32_t)state.bindings.size();
    state.info.pVertexBindingDescriptions = state.bindings.data();
    state.info.vertexAttributeDescriptionCount = (uint32_t)state.attributes.size();
    state.info.pVertexAttributeDescriptions = state.attributes.data();

    m_cache[key] = state;
    return &m_cache[key].info;
}
