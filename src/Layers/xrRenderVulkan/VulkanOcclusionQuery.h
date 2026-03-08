#pragma once

#include "VulkanHW.h"

class CVulkanOcclusionQuery
{
public:
    CVulkanOcclusionQuery();
    ~CVulkanOcclusionQuery();

    void Create(uint32_t count);
    void Destroy();

    void Begin(uint32_t index);
    void End(uint32_t index);
    uint64_t GetResult(uint32_t index);

    VkQueryPool GetPool() { return m_pool; }

private:
    VkQueryPool m_pool;
    uint32_t m_count;
};
