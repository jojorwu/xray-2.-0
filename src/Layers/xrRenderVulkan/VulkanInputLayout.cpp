#include "stdafx.h"
#include "VulkanInputLayout.h"

void CVulkanInputLayout::Convert(D3DVERTEXELEMENT9* dcl, xr_vector<VkVertexInputBindingDescription>& bindings, xr_vector<VkVertexInputAttributeDescription>& attributes)
{
    // Simplified mapping for now
    VkVertexInputBindingDescription binding = {};
    binding.binding = 0;
    binding.stride = 0; // Will be calculated
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings.push_back(binding);

    uint32_t offset = 0;
    for (int i = 0; dcl[i].Stream != 0xFF; ++i)
    {
        VkVertexInputAttributeDescription attr = {};
        attr.binding = dcl[i].Stream;
        attr.location = i;
        attr.offset = dcl[i].Offset;

        switch (dcl[i].Type)
        {
        case D3DDECLTYPE_FLOAT1: attr.format = VK_FORMAT_R32_SFLOAT; break;
        case D3DDECLTYPE_FLOAT2: attr.format = VK_FORMAT_R32G32_SFLOAT; break;
        case D3DDECLTYPE_FLOAT3: attr.format = VK_FORMAT_R32G32B32_SFLOAT; break;
        case D3DDECLTYPE_FLOAT4: attr.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
        case D3DDECLTYPE_D3DCOLOR: attr.format = VK_FORMAT_B8G8R8A8_UNORM; break;
        case D3DDECLTYPE_UBYTE4: attr.format = VK_FORMAT_R8G8B8A8_UINT; break;
        default: attr.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
        }

        attributes.push_back(attr);
    }
}
