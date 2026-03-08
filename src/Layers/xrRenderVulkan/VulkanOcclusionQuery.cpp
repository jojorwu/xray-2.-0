#include "stdafx.h"
#include "VulkanOcclusionQuery.h"
#include "VulkanBackend.h"

CVulkanOcclusionQuery::CVulkanOcclusionQuery()
{
    m_pool = VK_NULL_HANDLE;
    m_count = 0;
}

CVulkanOcclusionQuery::~CVulkanOcclusionQuery()
{
    Destroy();
}

void CVulkanOcclusionQuery::Create(uint32_t count)
{
    m_count = count;
    VkQueryPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
    createInfo.queryType = VK_QUERY_TYPE_OCCLUSION;
    createInfo.queryCount = count;

    if (vkCreateQueryPool(VulkanHW.GetDevice(), &createInfo, nullptr, &m_pool) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create occlusion query pool!");
    }
}

void CVulkanOcclusionQuery::Destroy()
{
    if (m_pool != VK_NULL_HANDLE)
    {
        vkDestroyQueryPool(VulkanHW.GetDevice(), m_pool, nullptr);
        m_pool = VK_NULL_HANDLE;
    }
}

void CVulkanOcclusionQuery::Begin(uint32_t index)
{
    vkCmdResetQueryPool(VulkanBackend.GetCurrentCommandBuffer(), m_pool, index, 1);
    vkCmdBeginQuery(VulkanBackend.GetCurrentCommandBuffer(), m_pool, index, 0);
}

void CVulkanOcclusionQuery::End(uint32_t index)
{
    vkCmdEndQuery(VulkanBackend.GetCurrentCommandBuffer(), m_pool, index);
}

uint64_t CVulkanOcclusionQuery::GetResult(uint32_t index)
{
    uint64_t result = 0;
    vkGetQueryPoolResults(VulkanHW.GetDevice(), m_pool, index, 1, sizeof(uint64_t), &result, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    return result;
}
