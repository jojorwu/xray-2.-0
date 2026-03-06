#include "stdafx.h"
#include "../../Include/xrAPI/xrAPI.h"
#include "VulkanRenderFactory.h"
#include "VulkanUIRender.h"

BOOL DllMainXrRenderVulkan(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		::Render = &VulkanRenderImpl;
		::RenderFactory = (dxRenderFactory*)(IRenderFactory*)&VulkanRenderFactoryImpl;
		// ::DU = &VulkanDUImpl; // Needs implementation
		::UIRender = &VulkanUIRenderImpl;
		// ::DRender = &VulkanDebugRenderImpl; // Needs implementation
		Msg("Vulkan: Renderer DLL attached");
		break ;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" {
    bool SupportsVulkanRendering();
};

bool SupportsVulkanRendering()
{
    return true; // TODO: Implement actual check
}
