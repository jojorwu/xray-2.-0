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
    void SetState(SState* state);
    void SetPipeline(VkPipeline pipeline, VkPipelineLayout layout, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    void SetDescriptorSet(VkDescriptorSet set, VkPipelineLayout layout, uint32_t firstSet = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);

    void set_RT(ID3DRenderTargetView* RT, u32 ID = 0);
    void set_ZB(ID3DDepthStencilView* ZB);

    void SetUniformBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
    void SetTexture(uint32_t binding, VkImageView view, VkSampler sampler);

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

    struct BindingState
    {
        xr_map<uint32_t, VkDescriptorBufferInfo> buffers;
        xr_map<uint32_t, VkDescriptorImageInfo> images;
        SState* state;
        bool dirty;
    } m_bindings;

    void ApplyBindings();

    VkPipelineLayout GetDefaultPipelineLayout() { return m_default_pipeline_layout; }
    VkDescriptorSetLayout GetDefaultDescriptorSetLayout() { return m_descriptor_set_layout; }

private:
    VkPipelineLayout m_current_pipeline_layout;
    VkPipelineLayout m_default_pipeline_layout;
    VkDescriptorSetLayout m_descriptor_set_layout; // Global for now

    void CreateDescriptorSetLayout();
    void CreatePipelineLayout();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void AllocateCommandBuffers();
    void CreateSyncPrimitives();
};

extern CVulkanBackend VulkanBackend;
