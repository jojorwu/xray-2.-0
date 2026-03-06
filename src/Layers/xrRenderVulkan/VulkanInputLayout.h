#pragma once

#include "VulkanHW.h"

class CVulkanInputLayout
{
public:
    static void Convert(D3DVERTEXELEMENT9* dcl, xr_vector<VkVertexInputBindingDescription>& bindings, xr_vector<VkVertexInputAttributeDescription>& attributes);
};
