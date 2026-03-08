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

    void SetVB(VkBuffer buffer, VkDeviceSize offset, uint32_t slot = 0);
    void SetIB(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType = VK_INDEX_TYPE_UINT16);
    void SetState(SState* state);
    void SetPipeline(VkPipeline pipeline, VkPipelineLayout layout, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    void SetDescriptorSet(VkDescriptorSet set, VkPipelineLayout layout, uint32_t firstSet = 0, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);

    void set_RT(ID3DRenderTargetView* RT, u32 ID = 0);
    void set_ZB(ID3DDepthStencilView* ZB);

    void set_VS(SVS* vs);
    void set_PS(SPS* ps);
    void set_Geometry(SGeometry* geom);
    void set_Viewport(const VkViewport& vp);
    void set_Scissor(const VkRect2D& scissor);
    void set_Topology(VkPrimitiveTopology topology);

    void SetComputePipeline(VkPipeline pipeline, VkPipelineLayout layout);
    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    uint32_t AllocateVB(uint32_t size, void** ptr);
    uint32_t AllocateIB(uint32_t size, void** ptr);

    void BeginQuery(uint32_t index);
    void EndQuery(uint32_t index);
    uint64_t GetQueryResult(uint32_t index);

    void EnsureRenderPass();
    void EndRenderPass();

    void CommitState();

    void SetUniformBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
    void SetPushConstants(uint32_t offset, uint32_t size, const void* data);
    void SetTexture(uint32_t binding, VkImageView view, VkSampler sampler);

    VkCommandBuffer GetCurrentCommandBuffer() { return m_command_buffers[m_current_image_index]; }
    VkCommandPool GetCommandPool() { return m_command_pool; }
    VkRenderPass GetRenderPass() { return m_render_pass; }
    VkExtent2D GetExtent() { return VulkanHW.GetSwapchainExtent(); }
    uint32_t GetCurrentImageIndex() { return m_current_image_index; }

private:
    VkRenderPass m_render_pass;
    xr_vector<VkFramebuffer> m_framebuffers;

    struct RenderPassKey
    {
        xr_array<VkFormat, 4> color_formats;
        VkFormat depth_format;
        bool operator<(const RenderPassKey& other) const
        {
            if (color_formats != other.color_formats) return color_formats < other.color_formats;
            return depth_format < other.depth_format;
        }
    };
    xr_map<RenderPassKey, VkRenderPass> m_render_pass_cache;

    struct FramebufferKey
    {
        VkRenderPass render_pass;
        xr_array<CVulkanRTView*, 4> color_views;
        CVulkanRTView* depth_view;
        VkExtent2D extent;
        bool operator<(const FramebufferKey& other) const
        {
            if (render_pass != other.render_pass) return render_pass < other.render_pass;
            if (color_views != other.color_views) return color_views < other.color_views;
            if (depth_view != other.depth_view) return depth_view < other.depth_view;
            if (extent.width != other.extent.width) return extent.width < other.extent.width;
            return extent.height < other.extent.height;
        }
    };
    xr_map<FramebufferKey, VkFramebuffer> m_framebuffer_cache;

    xr_array<ID3DRenderTargetView*, 4> m_pRT;
    ID3DDepthStencilView* m_pZB;

    SVS* m_pVS;
    SPS* m_pPS;
    SGeometry* m_pGeom;

    VkRenderPass m_active_render_pass;
    VkFramebuffer m_active_framebuffer;
    VkPipeline m_active_pipeline;
    xr_array<VkBuffer, 4> m_active_vbs;
    xr_array<VkDeviceSize, 4> m_active_offsets;
    VkBuffer m_active_ib;
    VkDeviceSize m_active_ib_offset;
    VkPrimitiveTopology m_topology;

    xr_array<VkBuffer, 4> m_pVB;
    xr_array<VkDeviceSize, 4> m_pVB_offsets;
    VkBuffer m_pIB;
    VkDeviceSize m_pIB_offset;
    VkIndexType m_pIB_type;

    bool m_is_render_pass_active;

    VkViewport m_viewport;
    VkRect2D m_scissor;
    bool m_viewport_dirty;
    bool m_scissor_dirty;
    bool m_pipeline_dirty;
    bool m_vbs_dirty;
    bool m_ib_dirty;

    VkCommandPool m_command_pool;
    xr_vector<VkCommandBuffer> m_command_buffers;

    CVulkanDynamicBuffer m_ub_ring;
    CVulkanDynamicBuffer m_pVB_stream;
    CVulkanDynamicBuffer m_pIB_stream;

    void CreateDynamicBuffers();
    void DestroyDynamicBuffers();
    VkDescriptorBufferInfo AllocateUniform(uint32_t size, const void* data);

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

    class CVulkanOcclusionQuery* m_occq;

private:
    VkPipelineLayout m_current_pipeline_layout;
    VkPipelineLayout m_default_pipeline_layout;
    VkPipelineLayout m_bindless_pipeline_layout;
    VkDescriptorSetLayout m_descriptor_set_layout; // Global for now

    VkRenderPass GetRenderPass(const RenderPassKey& key);
    VkFramebuffer GetFramebuffer(const FramebufferKey& key);

    void CreateDescriptorSetLayout();
    void CreatePipelineLayout();
    void CreateBindlessPipelineLayout();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateCommandPool();
    void AllocateCommandBuffers();
    void CreateSyncPrimitives();
};

extern CVulkanBackend VulkanBackend;
