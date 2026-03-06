#include "stdafx.h"
#include "VulkanInputLayout.h"

void CVulkanInputLayout::Convert(D3DVERTEXELEMENT9* dcl, xr_vector<VkVertexInputBindingDescription>& bindings, xr_vector<VkVertexInputAttributeDescription>& attributes)
{
    xr_map<uint32_t, uint32_t> strides;

    for (int i = 0; dcl[i].Stream != 0xFF; ++i)
    {
        VkVertexInputAttributeDescription attr = {};
        attr.binding = dcl[i].Stream;
        attr.location = i;
        attr.offset = dcl[i].Offset;

        uint32_t elementSize = 0;
        switch (dcl[i].Type)
        {
        case D3DDECLTYPE_FLOAT1: attr.format = VK_FORMAT_R32_SFLOAT; elementSize = 4; break;
        case D3DDECLTYPE_FLOAT2: attr.format = VK_FORMAT_R32G32_SFLOAT; elementSize = 8; break;
        case D3DDECLTYPE_FLOAT3: attr.format = VK_FORMAT_R32G32B32_SFLOAT; elementSize = 12; break;
        case D3DDECLTYPE_FLOAT4: attr.format = VK_FORMAT_R32G32B32A32_SFLOAT; elementSize = 16; break;
        case D3DDECLTYPE_D3DCOLOR: attr.format = VK_FORMAT_B8G8R8A8_UNORM; elementSize = 4; break;
        case D3DDECLTYPE_UBYTE4: attr.format = VK_FORMAT_R8G8B8A8_UINT; elementSize = 4; break;
        case D3DDECLTYPE_SHORT2: attr.format = VK_FORMAT_R16G16_SINT; elementSize = 4; break;
        case D3DDECLTYPE_SHORT4: attr.format = VK_FORMAT_R16G16B16A16_SINT; elementSize = 8; break;
        default: attr.format = VK_FORMAT_R32G32B32A32_SFLOAT; elementSize = 16; break;
        }

        attributes.push_back(attr);
        strides[dcl[i].Stream] = std::max(strides[dcl[i].Stream], (uint32_t)dcl[i].Offset + elementSize);
    }

    for (auto const& [stream, stride] : strides)
    {
        VkVertexInputBindingDescription binding = {};
        binding.binding = stream;
        binding.stride = stride;
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindings.push_back(binding);
    }
}
