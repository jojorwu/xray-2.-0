#include "stdafx.h"
#include "VulkanBackend.h"
#include "VulkanPipelineCache.h"
#include "VulkanDescriptorManager.h"
#include "VulkanOcclusionQuery.h"
#include "VulkanConstantBuffer.h"

CVulkanBackend VulkanBackend;

CVulkanBackend::CVulkanBackend()
{
    m_render_pass = VK_NULL_HANDLE;
    m_command_pool = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_image_available_semaphores[i] = VK_NULL_HANDLE;
        m_render_finished_semaphores[i] = VK_NULL_HANDLE;
        m_in_flight_fences[i] = VK_NULL_HANDLE;
    }
    m_current_frame = 0;
    m_current_image_index = 0;
    m_is_frame_started = false;
    m_bindings.dirty = false;

    m_pRT.fill(nullptr);
    m_pZB = nullptr;
    m_pVS = nullptr;
    m_pPS = nullptr;
    m_pGeom = nullptr;
    m_active_render_pass = VK_NULL_HANDLE;
    m_active_framebuffer = VK_NULL_HANDLE;
    m_active_pipeline = VK_NULL_HANDLE;
    m_active_vbs.fill(VK_NULL_HANDLE);
    m_active_offsets.fill(0);
    m_active_ib = VK_NULL_HANDLE;
    m_active_ib_offset = 0;
    m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_is_render_pass_active = false;
    m_viewport_dirty = false;
    m_occq = nullptr;
    m_scissor_dirty = false;

    m_pVB.fill(VK_NULL_HANDLE);
    m_pVB_offsets.fill(0);
    m_pIB = VK_NULL_HANDLE;
    m_pIB_offset = 0;
    m_pIB_type = VK_INDEX_TYPE_UINT16;
}

CVulkanBackend::~CVulkanBackend()
{
}

void CVulkanBackend::Create()
{
    CreateDescriptorSetLayout();
    CreatePipelineLayout();
    CreateRenderPass();
    CreateFramebuffers();
    CreateCommandPool();
    AllocateCommandBuffers();
    CreateSyncPrimitives();
    CreateDynamicBuffers();
    VulkanPipelineCache.Create();
    VulkanDescriptorManager.Create();
    CreateBindlessPipelineLayout();
}

void CVulkanBackend::Destroy()
{
    VulkanDescriptorManager.Destroy();
    VulkanPipelineCache.Destroy();
    VkDevice device = VulkanHW.GetDevice();

    if (m_default_pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, m_default_pipeline_layout, nullptr);

    if (m_descriptor_set_layout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, m_descriptor_set_layout, nullptr);

    if (m_bindless_pipeline_layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, m_bindless_pipeline_layout, nullptr);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (m_in_flight_fences[i] != VK_NULL_HANDLE)
            vkDestroyFence(device, m_in_flight_fences[i], nullptr);

        if (m_render_finished_semaphores[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(device, m_render_finished_semaphores[i], nullptr);

        if (m_image_available_semaphores[i] != VK_NULL_HANDLE)
            vkDestroySemaphore(device, m_image_available_semaphores[i], nullptr);
    }

    if (m_command_pool != VK_NULL_HANDLE)
        vkDestroyCommandPool(device, m_command_pool, nullptr);

    for (auto framebuffer : m_framebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    m_framebuffers.clear();

    if (m_render_pass != VK_NULL_HANDLE)
        vkDestroyRenderPass(device, m_render_pass, nullptr);

    DestroyDynamicBuffers();

    for (auto& it : m_render_pass_cache)
        vkDestroyRenderPass(device, it.second, nullptr);
    m_render_pass_cache.clear();

    for (auto& it : m_framebuffer_cache)
        vkDestroyFramebuffer(device, it.second, nullptr);
    m_framebuffer_cache.clear();
}

void CVulkanBackend::OnDeviceCreate()
{
    Create();
}

void CVulkanBackend::OnDeviceDestroy()
{
    if (m_occq) m_occq->Destroy();
    xr_delete(m_occq);
    Destroy();
}

bool CVulkanBackend::Begin()
{
    VkDevice device = VulkanHW.GetDevice();
    VkSwapchainKHR swapchain = VulkanHW.GetSwapchain();

    if (device == VK_NULL_HANDLE || swapchain == VK_NULL_HANDLE)
        return false;

    m_current_pipeline_layout = m_bindless_pipeline_layout ? m_bindless_pipeline_layout : m_default_pipeline_layout;

    vkWaitForFences(device, 1, &m_in_flight_fences[m_current_frame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, m_image_available_semaphores[m_current_frame], VK_NULL_HANDLE, &m_current_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        Msg("! Vulkan: Swapchain out of date on AcquireNextImageKHR");
        VulkanHW.RecreateSwapchain();
        return false;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        Msg("! Vulkan: Failed to acquire swapchain image!");
        return false;
    }

    vkResetFences(device, 1, &m_in_flight_fences[m_current_frame]);

    vkResetCommandBuffer(m_command_buffers[m_current_image_index], 0);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_command_buffers[m_current_image_index], &begin_info) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to begin recording command buffer!");
        return false;
    }

    m_is_frame_started = true;
    m_active_pipeline = VK_NULL_HANDLE;
    m_active_vbs.fill(VK_NULL_HANDLE);
    m_active_ib = VK_NULL_HANDLE;
    m_pipeline_dirty = true;
    m_vbs_dirty = true;
    m_ib_dirty = true;
    m_pRT.fill(nullptr);
    m_pRT[0] = VulkanHW.GetSwapchainRTView(m_current_image_index);
    m_pZB = VulkanHW.GetDepthRTView();
    return true;
}

void CVulkanBackend::SetVB(VkBuffer buffer, VkDeviceSize offset, uint32_t slot)
{
    if (m_pVB[slot] != buffer || m_pVB_offsets[slot] != offset)
    {
        m_pVB[slot] = buffer;
        m_pVB_offsets[slot] = offset;
        m_vbs_dirty = true;
    }
}

void CVulkanBackend::SetIB(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    if (m_pIB != buffer || m_pIB_offset != offset || m_pIB_type != indexType)
    {
        m_pIB = buffer;
        m_pIB_offset = offset;
        m_pIB_type = indexType;
        m_ib_dirty = true;
    }
}

void CVulkanBackend::SetState(SState* state)
{
    if (m_bindings.state != state)
    {
        m_bindings.state = state;
        m_bindings.dirty = true;
    }
}

void CVulkanBackend::SetPipeline(VkPipeline pipeline, VkPipelineLayout layout, VkPipelineBindPoint bindPoint)
{
    if (!m_is_frame_started)
        return;
    EnsureRenderPass();
    m_current_pipeline_layout = layout;
    vkCmdBindPipeline(m_command_buffers[m_current_image_index], bindPoint, pipeline);
}

void CVulkanBackend::SetDescriptorSet(VkDescriptorSet set, VkPipelineLayout layout, uint32_t firstSet, VkPipelineBindPoint bindPoint)
{
    if (!m_is_frame_started)
        return;
    EnsureRenderPass();
    vkCmdBindDescriptorSets(m_command_buffers[m_current_image_index], bindPoint, layout, firstSet, 1, &set, 0, nullptr);
}

void CVulkanBackend::SetUniformBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
{
    VkDescriptorBufferInfo info = { buffer, offset, range };
    m_bindings.buffers[binding] = info;
    m_bindings.dirty = true;
}

void CVulkanBackend::SetPushConstants(uint32_t offset, uint32_t size, const void* data)
{
    if (!m_is_frame_started)
        return;
    vkCmdPushConstants(m_command_buffers[m_current_image_index], m_current_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, offset, size, data);
}

void CVulkanBackend::SetTexture(uint32_t binding, VkImageView view, VkSampler sampler)
{
    if (m_bindings.images.count(binding) &&
        m_bindings.images[binding].imageView == view &&
        m_bindings.images[binding].sampler == sampler)
        return;

    VkDescriptorImageInfo info = { sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    m_bindings.images[binding] = info;
    m_bindings.dirty = true;
}

void CVulkanBackend::set_RT(ID3DRenderTargetView* RT, u32 ID)
{
    if (m_pRT[ID] != RT)
    {
        EndRenderPass();
        m_pRT[ID] = RT;
    }
}

void CVulkanBackend::set_ZB(ID3DDepthStencilView* ZB)
{
    if (m_pZB != ZB)
    {
        EndRenderPass();
        m_pZB = ZB;
    }
}

void CVulkanBackend::set_VS(SVS* vs)
{
    m_pVS = vs;
}

void CVulkanBackend::set_PS(SPS* ps)
{
    m_pPS = ps;
}

void CVulkanBackend::set_Geometry(SGeometry* geom)
{
    m_pGeom = geom;
}

void CVulkanBackend::set_Viewport(const VkViewport& vp)
{
    m_viewport = vp;
    m_viewport_dirty = true;
}

void CVulkanBackend::set_Scissor(const VkRect2D& scissor)
{
    m_scissor = scissor;
    m_scissor_dirty = true;
}

void CVulkanBackend::set_Topology(VkPrimitiveTopology topology)
{
    m_topology = topology;
}

void CVulkanBackend::SetComputePipeline(VkPipeline pipeline, VkPipelineLayout layout)
{
    if (!m_is_frame_started)
        return;
    m_current_pipeline_layout = layout;
    vkCmdBindPipeline(m_command_buffers[m_current_image_index], VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

void CVulkanBackend::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    if (!m_is_frame_started)
        return;
    // Potentially apply compute bindings here
    vkCmdDispatch(m_command_buffers[m_current_image_index], groupCountX, groupCountY, groupCountZ);
}

void CVulkanBackend::BeginQuery(uint32_t index)
{
    if (!m_occq)
    {
        m_occq = xr_new<CVulkanOcclusionQuery>();
        m_occq->Create(1024);
    }
    EnsureRenderPass();
    m_occq->Begin(index);
}

void CVulkanBackend::EndQuery(uint32_t index)
{
    if (m_occq) m_occq->End(index);
}

uint64_t CVulkanBackend::GetQueryResult(uint32_t index)
{
    if (m_occq) return m_occq->GetResult(index);
    return 1; // Assume visible if no query system
}

VkRenderPass CVulkanBackend::GetRenderPass(const RenderPassKey& key)
{
    auto it = m_render_pass_cache.find(key);
    if (it != m_render_pass_cache.end()) return it->second;

    xr_vector<VkAttachmentDescription> attachments;
    xr_vector<VkAttachmentReference> color_refs;

    for (u32 i = 0; i < 4; i++)
    {
        if (key.color_formats[i] == VK_FORMAT_UNDEFINED) continue;

        VkAttachmentDescription desc = {};
        desc.format = key.color_formats[i];
        desc.samples = VK_SAMPLE_COUNT_1_BIT;
        desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachments.push_back(desc);

        VkAttachmentReference ref = {};
        ref.attachment = (uint32_t)attachments.size() - 1;
        ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_refs.push_back(ref);
    }

    VkAttachmentReference depth_ref = {};
    bool has_depth = key.depth_format != VK_FORMAT_UNDEFINED;
    if (has_depth)
    {
        VkAttachmentDescription desc = {};
        desc.format = key.depth_format;
        desc.samples = VK_SAMPLE_COUNT_1_BIT;
        desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        desc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments.push_back(desc);

        depth_ref.attachment = (uint32_t)attachments.size() - 1;
        depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = (uint32_t)color_refs.size();
    subpass.pColorAttachments = color_refs.data();
    if (has_depth) subpass.pDepthStencilAttachment = &depth_ref;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = (uint32_t)attachments.size();
    info.pAttachments = attachments.data();
    info.subpassCount = 1;
    info.pSubpasses = &subpass;

    VkRenderPass rp;
    vkCreateRenderPass(VulkanHW.GetDevice(), &info, nullptr, &rp);
    m_render_pass_cache[key] = rp;
    return rp;
}

VkFramebuffer CVulkanBackend::GetFramebuffer(const FramebufferKey& key)
{
    auto it = m_framebuffer_cache.find(key);
    if (it != m_framebuffer_cache.end()) return it->second;

    xr_vector<VkImageView> attachments;
    for (u32 i = 0; i < 4; i++)
    {
        if (key.color_views[i])
            attachments.push_back(key.color_views[i]->view);
    }
    if (key.depth_view)
        attachments.push_back(key.depth_view->view);

    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = key.render_pass;
    info.attachmentCount = (uint32_t)attachments.size();
    info.pAttachments = attachments.data();
    info.width = key.extent.width;
    info.height = key.extent.height;
    info.layers = 1;

    VkFramebuffer fb;
    vkCreateFramebuffer(VulkanHW.GetDevice(), &info, nullptr, &fb);
    m_framebuffer_cache[key] = fb;
    return fb;
}

void CVulkanBackend::TransitionRT(CVulkanRTView* rt, VkImageLayout new_layout)
{
    if (!rt || rt->current_layout == new_layout) return;

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = rt->current_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = rt->image;

    if (rt->format == VK_FORMAT_D24_UNORM_S8_UINT || rt->format == VK_FORMAT_D32_SFLOAT_S8_UINT)
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    else if (rt->format == VK_FORMAT_D32_SFLOAT || rt->format == VK_FORMAT_D16_UNORM)
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkCmdPipelineBarrier(m_command_buffers[m_current_image_index], srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    rt->current_layout = new_layout;
}

void CVulkanBackend::EnsureRenderPass()
{
    if (m_is_render_pass_active) return;

    RenderPassKey rp_key;
    FramebufferKey fb_key;
    fb_key.extent = { 0, 0 };

    for (u32 i = 0; i < 4; i++)
    {
        if (m_pRT[i])
        {
            rp_key.color_formats[i] = m_pRT[i]->format;
            fb_key.color_views[i] = m_pRT[i];
            fb_key.extent = m_pRT[i]->extent;
            TransitionRT(m_pRT[i], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
        else
        {
            rp_key.color_formats[i] = VK_FORMAT_UNDEFINED;
            fb_key.color_views[i] = nullptr;
        }
    }

    if (m_pZB)
    {
        rp_key.depth_format = m_pZB->format;
        fb_key.depth_view = m_pZB;
        if (fb_key.extent.width == 0) fb_key.extent = m_pZB->extent;
        TransitionRT(m_pZB, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
    else
    {
        rp_key.depth_format = VK_FORMAT_UNDEFINED;
        fb_key.depth_view = nullptr;
    }

    m_active_render_pass = GetRenderPass(rp_key);
    fb_key.render_pass = m_active_render_pass;
    m_active_framebuffer = GetFramebuffer(fb_key);

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m_active_render_pass;
    render_pass_info.framebuffer = m_active_framebuffer;
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = fb_key.extent;

    render_pass_info.clearValueCount = 0;
    render_pass_info.pClearValues = nullptr;

    vkCmdBeginRenderPass(m_command_buffers[m_current_image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    m_viewport.x = 0.0f;
    m_viewport.y = 0.0f;
    m_viewport.width = (float)render_pass_info.renderArea.extent.width;
    m_viewport.height = (float)render_pass_info.renderArea.extent.height;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;
    m_viewport_dirty = true;

    m_scissor.offset = { 0, 0 };
    m_scissor.extent = render_pass_info.renderArea.extent;
    m_scissor_dirty = true;

    m_is_render_pass_active = true;
}

void CVulkanBackend::EndRenderPass()
{
    if (!m_is_render_pass_active) return;
    vkCmdEndRenderPass(m_command_buffers[m_current_image_index]);
    m_is_render_pass_active = false;
}

void CVulkanBackend::ApplyBindings()
{
    DescriptorSetKey key;
    key.buffers = m_bindings.buffers;
    key.images = m_bindings.images;
    key.ComputeHash();

    VkDescriptorSet set = VulkanDescriptorManager.GetDescriptorSet(m_descriptor_set_layout, key);

    if (set != VK_NULL_HANDLE)
    {
        xr_vector<uint32_t> dynamic_offsets;
        for (auto const& [binding, info] : m_bindings.buffers)
            dynamic_offsets.push_back((uint32_t)info.offset);

        vkCmdBindDescriptorSets(m_command_buffers[m_current_image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_current_pipeline_layout, 0, 1, &set, (uint32_t)dynamic_offsets.size(), dynamic_offsets.data());
    }

    if (m_bindless_pipeline_layout)
    {
        VkDescriptorSet bindless_set = VulkanDescriptorManager.GetBindlessSet();
        vkCmdBindDescriptorSets(m_command_buffers[m_current_image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_current_pipeline_layout, 1, 1, &bindless_set, 0, nullptr);
    }

    m_bindings.dirty = false;
}

void CVulkanBackend::CommitState()
{
    EnsureRenderPass();

    if (m_viewport_dirty)
    {
        vkCmdSetViewport(m_command_buffers[m_current_image_index], 0, 1, &m_viewport);
        m_viewport_dirty = false;
    }
    if (m_scissor_dirty)
    {
        vkCmdSetScissor(m_command_buffers[m_current_image_index], 0, 1, &m_scissor);
        m_scissor_dirty = false;
    }

    ApplyBindings();

    if (!m_pVS || !m_pPS) return;

    PipelineStateKey key = {};
    key.vs = m_pVS->vs;
    key.ps = m_pPS->ps;
    key.renderPass = m_active_render_pass;
    key.layout = m_current_pipeline_layout;
    key.topology = m_topology;
    key.inputLayoutHash = m_pGeom ? (uint32_t)(intptr_t)m_pGeom->dcl : 0;

    key.colorAttachmentCount = 0;
    for (u32 i = 0; i < 4; i++)
    {
        if (m_pRT[i]) key.colorAttachmentCount = i + 1;
    }

    // Handle uniform updates
    if (m_bindings.dirty)
    {
        // Update shader constants
        if (m_pVS)
        {
            for (auto& cb : m_pVS->constants.m_CBTable)
            {
                VkDescriptorBufferInfo info = cb.second->VulkanUpdate();
                SetUniformBuffer(cb.first, info.buffer, info.offset, info.range);
            }
        }
        if (m_pPS)
        {
            for (auto& cb : m_pPS->constants.m_CBTable)
            {
                VkDescriptorBufferInfo info = cb.second->VulkanUpdate();
                SetUniformBuffer(cb.first, info.buffer, info.offset, info.range);
            }
        }
    }

    if (m_bindings.state)
    {
        CVulkanState::ConvertRasterizer(m_bindings.state->state_code, key.rasterizer);
        CVulkanState::ConvertDepthStencil(m_bindings.state->state_code, key.depthStencil, key.front, key.back);
        xr_vector<VkPipelineColorBlendAttachmentState> blendAttachments;
        VkPipelineColorBlendStateCreateInfo blendInfo = {};
        CVulkanState::ConvertBlend(m_bindings.state->state_code, blendInfo, blendAttachments);
        for (u32 i = 0; i < 4; i++)
        {
            if (i < blendAttachments.size())
                key.blendAttachments[i] = blendAttachments[i];
            else
            {
                key.blendAttachments[i] = {};
                key.blendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }
        }
    }

    VkPipeline pipeline = VulkanPipelineCache.GetPipeline(key);
    if (pipeline != VK_NULL_HANDLE && (pipeline != m_active_pipeline || m_pipeline_dirty))
    {
        vkCmdBindPipeline(m_command_buffers[m_current_image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        m_active_pipeline = pipeline;
        m_pipeline_dirty = false;
    }

    if (m_pGeom)
    {
        if (m_pGeom->vb && m_pGeom->vb->m_buffer != VK_NULL_HANDLE)
            SetVB(m_pGeom->vb->m_buffer, 0, 0);
        if (m_pGeom->ib && m_pGeom->ib->m_buffer != VK_NULL_HANDLE)
            SetIB(m_pGeom->ib->m_buffer, 0, VK_INDEX_TYPE_UINT16);
    }

    if (m_vbs_dirty)
    {
        for (u32 i = 0; i < 4; i++)
        {
            if (m_pVB[i] != m_active_vbs[i] || m_pVB_offsets[i] != m_active_offsets[i])
            {
                if (m_pVB[i])
                    vkCmdBindVertexBuffers(m_command_buffers[m_current_image_index], i, 1, &m_pVB[i], &m_pVB_offsets[i]);
                m_active_vbs[i] = m_pVB[i];
                m_active_offsets[i] = m_pVB_offsets[i];
            }
        }
        m_vbs_dirty = false;
    }

    if (m_ib_dirty)
    {
        if (m_pIB != m_active_ib || m_pIB_offset != m_active_ib_offset)
        {
            if (m_pIB)
                vkCmdBindIndexBuffer(m_command_buffers[m_current_image_index], m_pIB, m_pIB_offset, m_pIB_type);
            m_active_ib = m_pIB;
            m_active_ib_offset = m_pIB_offset;
        }
        m_ib_dirty = false;
    }

    if (m_pVB_stream.m_buffer != m_active_vbs[1]) // Bind dynamic stream to slot 1 for now
    {
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(m_command_buffers[m_current_image_index], 1, 1, &m_pVB_stream.m_buffer, &offset);
        m_active_vbs[1] = m_pVB_stream.m_buffer;
        m_active_offsets[1] = offset;
    }
}

void CVulkanBackend::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    if (!m_is_frame_started)
        return;

    CommitState();
    vkCmdDraw(m_command_buffers[m_current_image_index], vertexCount, instanceCount, firstVertex, firstInstance);
}

void CVulkanBackend::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    if (!m_is_frame_started)
        return;

    CommitState();
    vkCmdDrawIndexed(m_command_buffers[m_current_image_index], indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CVulkanBackend::DrawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    if (!m_is_frame_started)
        return;

    CommitState();
    vkCmdDrawIndexedIndirect(m_command_buffers[m_current_image_index], buffer, offset, drawCount, stride);
}

void CVulkanBackend::Clear()
{
    if (!m_is_frame_started)
        return;

    EnsureRenderPass();

    xr_vector<VkClearAttachment> attachments;
    for (u32 i = 0; i < 4; i++)
    {
        if (m_pRT[i])
        {
            VkClearAttachment clear = {};
            clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            clear.colorAttachment = i;
            clear.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            attachments.push_back(clear);
        }
    }

    if (m_pZB)
    {
        VkClearAttachment clear = {};
        clear.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        clear.clearValue.depthStencil = { 1.0f, 0 };
        attachments.push_back(clear);
    }

    if (attachments.empty()) return;

    VkClearRect rect = {};
    rect.rect.offset = { 0, 0 };
    rect.rect.extent = VulkanHW.GetSwapchainExtent();
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;

    vkCmdClearAttachments(m_command_buffers[m_current_image_index], (uint32_t)attachments.size(), attachments.data(), 1, &rect);
}

void CVulkanBackend::ClearTarget()
{
    if (!m_is_frame_started)
        return;

    EnsureRenderPass();

    xr_vector<VkClearAttachment> attachments;
    for (u32 i = 0; i < 4; i++)
    {
        if (m_pRT[i])
        {
            VkClearAttachment clear = {};
            clear.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            clear.colorAttachment = i;
            clear.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            attachments.push_back(clear);
        }
    }

    if (attachments.empty()) return;

    VkClearRect rect = {};
    rect.rect.offset = { 0, 0 };
    rect.rect.extent = VulkanHW.GetSwapchainExtent();
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;

    vkCmdClearAttachments(m_command_buffers[m_current_image_index], (uint32_t)attachments.size(), attachments.data(), 1, &rect);
}

void CVulkanBackend::End()
{
    if (!m_is_frame_started)
        return;

    EndRenderPass();

    for (u32 i = 0; i < 4; i++)
    {
        if (m_pRT[i] == VulkanHW.GetSwapchainRTView(m_current_image_index))
        {
            TransitionRT(m_pRT[i], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        }
    }

    m_is_frame_started = false;

    VkDevice device = VulkanHW.GetDevice();

    if (vkEndCommandBuffer(m_command_buffers[m_current_image_index]) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to end command buffer recording!");
        return;
    }

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = { m_image_available_semaphores[m_current_frame] };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffers[m_current_image_index];

    VkSemaphore signal_semaphores[] = { m_render_finished_semaphores[m_current_frame] };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(VulkanHW.GetGraphicsQueue(), 1, &submit_info, m_in_flight_fences[m_current_frame]) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to submit graphics queue!");
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    VkSwapchainKHR swapchains[] = { VulkanHW.GetSwapchain() };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &m_current_image_index;

    VkResult result = vkQueuePresentKHR(VulkanHW.GetPresentQueue(), &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        Msg("! Vulkan: Swapchain out of date on QueuePresentKHR");
        VulkanHW.RecreateSwapchain();
    }

    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    m_ub_ring.Reset();
    m_pVB_stream.Reset();
    m_pIB_stream.Reset();
}

void CVulkanBackend::CreateDescriptorSetLayout()
{
    xr_vector<VkDescriptorSetLayoutBinding> bindings;

    // Uniform Buffers (Slot 0..3) - Use dynamic offsets to reduce descriptor updates
    for (uint32_t i = 0; i < 4; i++)
    {
        VkDescriptorSetLayoutBinding b = { i, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr };
        bindings.push_back(b);
    }

    // Textures (Slot 4..19)
    for (uint32_t i = 0; i < 16; i++)
    {
        VkDescriptorSetLayoutBinding b = { 4 + i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr };
        bindings.push_back(b);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = (uint32_t)bindings.size();
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(VulkanHW.GetDevice(), &layoutInfo, nullptr, &m_descriptor_set_layout) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create descriptor set layout!");
    }
}

void CVulkanBackend::CreatePipelineLayout()
{
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = 128; // Standard push constant size

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptor_set_layout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(VulkanHW.GetDevice(), &pipelineLayoutInfo, nullptr, &m_default_pipeline_layout) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create pipeline layout!");
    }
}

void CVulkanBackend::CreateBindlessPipelineLayout()
{
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = 128;

    xr_array<VkDescriptorSetLayout, 2> layouts = { m_descriptor_set_layout, VulkanDescriptorManager.GetBindlessLayout() };
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = (uint32_t)layouts.size();
    pipelineLayoutInfo.pSetLayouts = layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(VulkanHW.GetDevice(), &pipelineLayoutInfo, nullptr, &m_bindless_pipeline_layout) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create bindless pipeline layout!");
    }
}

void CVulkanBackend::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = VulkanHW.GetSwapchainFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = VulkanHW.GetDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    xr_array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (uint32_t)attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    vkCreateRenderPass(VulkanHW.GetDevice(), &renderPassInfo, nullptr, &m_render_pass);
}

void CVulkanBackend::CreateFramebuffers()
{
    const auto& imageViews = VulkanHW.GetSwapchainImageViews();
    m_framebuffers.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); i++)
    {
        xr_array<VkImageView, 2> attachments = { imageViews[i], VulkanHW.GetDepthImageView() };
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_render_pass;
        framebufferInfo.attachmentCount = (uint32_t)attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = VulkanHW.GetSwapchainExtent().width;
        framebufferInfo.height = VulkanHW.GetSwapchainExtent().height;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(VulkanHW.GetDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]);
    }
}

void CVulkanBackend::CreateCommandPool()
{
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = VulkanHW.GetGraphicsFamily();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCreateCommandPool(VulkanHW.GetDevice(), &poolInfo, nullptr, &m_command_pool);
}

void CVulkanBackend::AllocateCommandBuffers()
{
    m_command_buffers.resize(m_framebuffers.size());
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_command_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_command_buffers.size();

    vkAllocateCommandBuffers(VulkanHW.GetDevice(), &allocInfo, m_command_buffers.data());
}

void CVulkanBackend::CreateDynamicBuffers()
{
    m_ub_ring.Create(1024 * 1024 * 4, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_pVB_stream.Create(1024 * 1024 * 4, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    m_pIB_stream.Create(1024 * 1024 * 1, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void CVulkanBackend::DestroyDynamicBuffers()
{
    m_pIB_stream.Destroy();
    m_pVB_stream.Destroy();
    m_ub_ring.Destroy();
}

VkDescriptorBufferInfo CVulkanBackend::AllocateUniform(uint32_t size, const void* data)
{
    void* ptr;
    uint32_t offset = m_ub_ring.Alloc(size, &ptr);
    if (offset != 0xFFFFFFFF)
    {
        memcpy(ptr, data, size);
        return { m_ub_ring.m_buffer, offset, size };
    }
    return { VK_NULL_HANDLE, 0, 0 };
}

uint32_t CVulkanBackend::AllocateVB(uint32_t size, void** ptr)
{
    return m_pVB_stream.Alloc(size, ptr);
}

uint32_t CVulkanBackend::AllocateIB(uint32_t size, void** ptr)
{
    return m_pIB_stream.Alloc(size, ptr);
}

void CVulkanBackend::CreateSyncPrimitives()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkDevice device = VulkanHW.GetDevice();
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_image_available_semaphores[i]);
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_render_finished_semaphores[i]);
        vkCreateFence(device, &fenceInfo, nullptr, &m_in_flight_fences[i]);
    }
}
