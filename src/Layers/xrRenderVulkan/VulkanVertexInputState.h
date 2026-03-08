#pragma once

#include "VulkanHW.h"

struct VertexInputStateKey
{
    xr_vector<VkVertexInputBindingDescription> bindings;
    xr_vector<VkVertexInputAttributeDescription> attributes;

    bool operator<(const VertexInputStateKey& other) const
    {
        if (bindings.size() != other.bindings.size()) return bindings.size() < other.bindings.size();
        if (attributes.size() != other.attributes.size()) return attributes.size() < other.attributes.size();

        int res;
        res = memcmp(bindings.data(), other.bindings.data(), bindings.size() * sizeof(VkVertexInputBindingDescription));
        if (res != 0) return res < 0;
        return memcmp(attributes.data(), other.attributes.data(), attributes.size() * sizeof(VkVertexInputAttributeDescription)) < 0;
    }
};

class CVulkanVertexInputCache
{
public:
    CVulkanVertexInputCache();
    ~CVulkanVertexInputCache();

    VkPipelineVertexInputStateCreateInfo* GetState(D3DVERTEXELEMENT9* dcl);

private:
    struct CachedState
    {
        xr_vector<VkVertexInputBindingDescription> bindings;
        xr_vector<VkVertexInputAttributeDescription> attributes;
        VkPipelineVertexInputStateCreateInfo info;
    };
    xr_map<uint32_t, CachedState> m_cache; // Key is decl pointer or hash
};

extern CVulkanVertexInputCache VulkanVertexInputCache;
