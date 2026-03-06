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

    void Clear();
    void ClearTarget();

    void SetVB(VkBuffer buffer, VkDeviceSize offset);
    void SetIB(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
    void SetPipeline(VkPipeline pipeline, VkPipelineLayout layout, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    void SetDescriptorSet(VkDescriptorSet set, VkPipelineLayout layout, uint32_t firstSet = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);

    VkCommandBuffer GetCurrentCommandBuffer() { return m_command_buffers[m_current_image_index]; }
    VkCommandPool GetCommandPool() { return m_command_pool; }
    VkRenderPass GetRenderPass() { return m_render_pass; }
    VkExtent2D GetExtent() { return VulkanHW.GetSwapchainExtent(); }
    uint32_t GetCurrentImageIndex() { return m_current_image_index; }

private:
    VkRenderPass m_render_pass;
    xr_vector<VkFramebuffer> m_framebuffers;
    VkCommandPool m_command_pool;
    xr_vector<VkCommandBuffer> m_command_buffers;

    static const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    xr_array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_image_available_semaphores;
    xr_array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_render_finished_semaphores;
    xr_array<VkFence, MAX_FRAMES_IN_FLIGHT> m_in_flight_fences;

    uint32_t m_current_frame;
    uint32_t m_current_image_index;
    bool m_is_frame_started;

    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void AllocateCommandBuffers();
    void CreateSyncPrimitives();
};

extern CVulkanBackend VulkanBackend;
