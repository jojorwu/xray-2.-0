#pragma once

#include "VulkanHW.h"

class CVulkanBackend
{
public:
    CVulkanBackend();
    ~CVulkanBackend();

    void Create();
    void Destroy();

    void OnDeviceCreate();
    void OnDeviceDestroy();

    bool Begin();
    void End();

    void SetVB(VkBuffer buffer, VkDeviceSize offset);
    void SetIB(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
    void SetPipeline(VkPipeline pipeline, VkPipelineLayout layout, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    void SetDescriptorSet(VkDescriptorSet set, VkPipelineLayout layout, uint32_t firstSet = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    VkCommandBuffer GetCurrentCommandBuffer() { return m_command_buffers[m_current_image_index]; }
    VkRenderPass GetRenderPass() { return m_render_pass; }
    VkExtent2D GetExtent() { return VulkanHW.GetSwapchainExtent(); }
    uint32_t GetCurrentImageIndex() { return m_current_image_index; }

private:
    VkRenderPass m_render_pass;
    xr_vector<VkFramebuffer> m_framebuffers;
    VkCommandPool m_command_pool;
    xr_vector<VkCommandBuffer> m_command_buffers;
    VkSemaphore m_image_available_semaphore;
    VkSemaphore m_render_finished_semaphore;
    VkFence m_in_flight_fence;
    uint32_t m_current_image_index;
    bool m_is_frame_started;

    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void AllocateCommandBuffers();
    void CreateSyncPrimitives();
};

extern CVulkanBackend VulkanBackend;
