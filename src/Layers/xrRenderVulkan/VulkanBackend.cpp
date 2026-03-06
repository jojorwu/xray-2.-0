#include "stdafx.h"
#include "VulkanBackend.h"

CVulkanBackend VulkanBackend;

CVulkanBackend::CVulkanBackend()
{
    m_render_pass = VK_NULL_HANDLE;
    m_command_pool = VK_NULL_HANDLE;
    m_image_available_semaphore = VK_NULL_HANDLE;
    m_render_finished_semaphore = VK_NULL_HANDLE;
    m_in_flight_fence = VK_NULL_HANDLE;
    m_current_image_index = 0;
    m_is_frame_started = false;
}

CVulkanBackend::~CVulkanBackend()
{
}

void CVulkanBackend::Create()
{
    CreateRenderPass();
    CreateFramebuffers();
    CreateCommandPool();
    AllocateCommandBuffers();
    CreateSyncPrimitives();
}

void CVulkanBackend::Destroy()
{
    VkDevice device = VulkanHW.GetDevice();

    if (m_in_flight_fence != VK_NULL_HANDLE)
        vkDestroyFence(device, m_in_flight_fence, nullptr);

    if (m_render_finished_semaphore != VK_NULL_HANDLE)
        vkDestroySemaphore(device, m_render_finished_semaphore, nullptr);

    if (m_image_available_semaphore != VK_NULL_HANDLE)
        vkDestroySemaphore(device, m_image_available_semaphore, nullptr);

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

    vkWaitForFences(device, 1, &m_in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &m_in_flight_fence);

    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, m_image_available_semaphore, VK_NULL_HANDLE, &m_current_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        Msg("! Vulkan: Swapchain out of date on AcquireNextImageKHR");
        return false;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        Msg("! Vulkan: Failed to acquire swapchain image!");
        return false;
    }

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

    VkClearValue clear_color = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(m_command_buffers[m_current_image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    m_is_frame_started = true;
    return true;
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

    VkSemaphore wait_semaphores[] = { m_image_available_semaphore };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffers[m_current_image_index];

    VkSemaphore signal_semaphores[] = { m_render_finished_semaphore };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkQueueSubmit(VulkanHW.GetGraphicsQueue(), 1, &submit_info, m_in_flight_fence);

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

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
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
        VkImageView attachments[] = { imageViews[i] };
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_render_pass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
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
    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_image_available_semaphore);
    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_render_finished_semaphore);
    vkCreateFence(device, &fenceInfo, nullptr, &m_in_flight_fence);
}
