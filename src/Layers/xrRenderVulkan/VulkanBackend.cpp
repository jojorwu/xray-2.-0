#include "stdafx.h"
#include "VulkanBackend.h"
#include "VulkanPipelineCache.h"
#include "VulkanDescriptorManager.h"

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
    VulkanPipelineCache.Create();
    VulkanDescriptorManager.Create();
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
}

void CVulkanBackend::OnDeviceCreate()
{
    Create();
}

void CVulkanBackend::OnDeviceDestroy()
{
    Destroy();
}

bool CVulkanBackend::Begin()
{
    VkDevice device = VulkanHW.GetDevice();
    VkSwapchainKHR swapchain = VulkanHW.GetSwapchain();

    if (device == VK_NULL_HANDLE || swapchain == VK_NULL_HANDLE)
        return false;

    m_current_pipeline_layout = m_default_pipeline_layout;

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

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m_render_pass;
    render_pass_info.framebuffer = m_framebuffers[m_current_image_index];
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = VulkanHW.GetSwapchainExtent();

    xr_array<VkClearValue, 2> clear_values;
    clear_values[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clear_values[1].depthStencil = { 1.0f, 0 };

    render_pass_info.clearValueCount = (uint32_t)clear_values.size();
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(m_command_buffers[m_current_image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)render_pass_info.renderArea.extent.width;
    viewport.height = (float)render_pass_info.renderArea.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_command_buffers[m_current_image_index], 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = render_pass_info.renderArea.extent;
    vkCmdSetScissor(m_command_buffers[m_current_image_index], 0, 1, &scissor);

    m_is_frame_started = true;
    return true;
}

void CVulkanBackend::SetVB(VkBuffer buffer, VkDeviceSize offset)
{
    if (!m_is_frame_started)
        return;

    vkCmdBindVertexBuffers(m_command_buffers[m_current_image_index], 0, 1, &buffer, &offset);
}

void CVulkanBackend::SetIB(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
    if (!m_is_frame_started)
        return;

    vkCmdBindIndexBuffer(m_command_buffers[m_current_image_index], buffer, offset, indexType);
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

    m_current_pipeline_layout = layout;
    vkCmdBindPipeline(m_command_buffers[m_current_image_index], bindPoint, pipeline);
}

void CVulkanBackend::SetDescriptorSet(VkDescriptorSet set, VkPipelineLayout layout, uint32_t firstSet, VkPipelineBindPoint bindPoint)
{
    if (!m_is_frame_started)
        return;

    vkCmdBindDescriptorSets(m_command_buffers[m_current_image_index], bindPoint, layout, firstSet, 1, &set, 0, nullptr);
}

void CVulkanBackend::SetUniformBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
{
    VkDescriptorBufferInfo info = { buffer, offset, range };
    m_bindings.buffers[binding] = info;
    m_bindings.dirty = true;
}

void CVulkanBackend::SetTexture(uint32_t binding, VkImageView view, VkSampler sampler)
{
    VkDescriptorImageInfo info = { sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    m_bindings.images[binding] = info;
    m_bindings.dirty = true;
}

void CVulkanBackend::set_RT(ID3DRenderTargetView* RT, u32 ID)
{
    // TODO: Implement RT switching logic
    // This will likely involve ending current render pass and starting a new one
}

void CVulkanBackend::set_ZB(ID3DDepthStencilView* ZB)
{
    // TODO: Implement ZB switching logic
}

void CVulkanBackend::ApplyBindings()
{
    if (!m_bindings.dirty) return;

    DescriptorSetKey key;
    key.buffers = m_bindings.buffers;
    key.images = m_bindings.images;

    VkDescriptorSet set = VulkanDescriptorManager.GetDescriptorSet(m_descriptor_set_layout, key);

    if (set != VK_NULL_HANDLE)
    {
        vkCmdBindDescriptorSets(m_command_buffers[m_current_image_index], VK_PIPELINE_BIND_POINT_GRAPHICS, m_current_pipeline_layout, 0, 1, &set, 0, nullptr);
    }

    m_bindings.dirty = false;
}

void CVulkanBackend::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    if (!m_is_frame_started)
        return;

    ApplyBindings();
    vkCmdDraw(m_command_buffers[m_current_image_index], vertexCount, instanceCount, firstVertex, firstInstance);
}

void CVulkanBackend::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    if (!m_is_frame_started)
        return;

    ApplyBindings();
    vkCmdDrawIndexed(m_command_buffers[m_current_image_index], indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CVulkanBackend::Clear()
{
    if (!m_is_frame_started)
        return;

    VkClearAttachment attachments[2] = {};
    attachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachments[0].colorAttachment = 0;
    attachments[0].clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

    attachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    attachments[1].clearValue.depthStencil = { 1.0f, 0 };

    VkClearRect rect = {};
    rect.rect.offset = { 0, 0 };
    rect.rect.extent = VulkanHW.GetSwapchainExtent();
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;

    vkCmdClearAttachments(m_command_buffers[m_current_image_index], 2, attachments, 1, &rect);
}

void CVulkanBackend::ClearTarget()
{
    if (!m_is_frame_started)
        return;

    VkClearAttachment attachment = {};
    attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    attachment.colorAttachment = 0;
    attachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

    VkClearRect rect = {};
    rect.rect.offset = { 0, 0 };
    rect.rect.extent = VulkanHW.GetSwapchainExtent();
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;

    vkCmdClearAttachments(m_command_buffers[m_current_image_index], 1, &attachment, 1, &rect);
}

void CVulkanBackend::End()
{
    if (!m_is_frame_started)
        return;

    m_is_frame_started = false;

    VkDevice device = VulkanHW.GetDevice();

    vkCmdEndRenderPass(m_command_buffers[m_current_image_index]);
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
}

void CVulkanBackend::CreateDescriptorSetLayout()
{
    xr_vector<VkDescriptorSetLayoutBinding> bindings;

    // Uniform Buffers (Slot 0..3)
    for (uint32_t i = 0; i < 4; i++)
    {
        VkDescriptorSetLayoutBinding b = { i, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr };
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
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptor_set_layout;

    if (vkCreatePipelineLayout(VulkanHW.GetDevice(), &pipelineLayoutInfo, nullptr, &m_default_pipeline_layout) != VK_SUCCESS)
    {
        Msg("! Vulkan: Failed to create pipeline layout!");
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
