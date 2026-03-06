#include "stdafx.h"
#include "VulkanRenderFactory.h"
#include "VulkanRenderDeviceRender.h"
#include "VulkanEnvironmentRender.h"
#include "VulkanUIShader.h"

CVulkanRenderFactory VulkanRenderFactoryImpl;

#define RENDER_FACTORY_IMPLEMENT(Class) \
	I##Class* CVulkanRenderFactory::Create##Class() \
{ \
	return nullptr; \
} \
	void CVulkanRenderFactory::Destroy##Class(I##Class *pObject)\
{ \
}

RENDER_FACTORY_IMPLEMENT(UISequenceVideoItem)

IUIShader* CVulkanRenderFactory::CreateUIShader()
{
	return xr_new<CVulkanUIShader>();
}

void CVulkanRenderFactory::DestroyUIShader(IUIShader* pObject)
{
	xr_delete((CVulkanUIShader*&)pObject);
}

RENDER_FACTORY_IMPLEMENT(StatGraphRender)
RENDER_FACTORY_IMPLEMENT(ConsoleRender)

IRenderDeviceRender* CVulkanRenderFactory::CreateRenderDeviceRender()
{
	return xr_new<CVulkanRenderDeviceRender>();
}

void CVulkanRenderFactory::DestroyRenderDeviceRender(IRenderDeviceRender* pObject)
{
	xr_delete((CVulkanRenderDeviceRender*&)pObject);
}

#ifdef DEBUG
RENDER_FACTORY_IMPLEMENT(ObjectSpaceRender)
#endif

RENDER_FACTORY_IMPLEMENT(ApplicationRender)
RENDER_FACTORY_IMPLEMENT(WallMarkArray)
RENDER_FACTORY_IMPLEMENT(StatsRender)

IFlareRender* CVulkanRenderFactory::CreateFlareRender()
{
	return nullptr;
}

void CVulkanRenderFactory::DestroyFlareRender(IFlareRender* pObject)
{
}

RENDER_FACTORY_IMPLEMENT(ThunderboltRender)
RENDER_FACTORY_IMPLEMENT(ThunderboltDescRender)
RENDER_FACTORY_IMPLEMENT(RainRender)
RENDER_FACTORY_IMPLEMENT(LensFlareRender)
RENDER_FACTORY_IMPLEMENT(ImGuiRender)
IEnvironmentRender* CVulkanRenderFactory::CreateEnvironmentRender()
{
	return xr_new<CVulkanEnvironmentRender>();
}

void CVulkanRenderFactory::DestroyEnvironmentRender(IEnvironmentRender* pObject)
{
	xr_delete((CVulkanEnvironmentRender*&)pObject);
}

IEnvDescriptorMixerRender* CVulkanRenderFactory::CreateEnvDescriptorMixerRender()
{
	return xr_new<CVulkanEnvDescriptorMixerRender>();
}

void CVulkanRenderFactory::DestroyEnvDescriptorMixerRender(IEnvDescriptorMixerRender* pObject)
{
	xr_delete((CVulkanEnvDescriptorMixerRender*&)pObject);
}

IEnvDescriptorRender* CVulkanRenderFactory::CreateEnvDescriptorRender()
{
	return xr_new<CVulkanEnvDescriptorRender>();
}

void CVulkanRenderFactory::DestroyEnvDescriptorRender(IEnvDescriptorRender* pObject)
{
	xr_delete((CVulkanEnvDescriptorRender*&)pObject);
}
RENDER_FACTORY_IMPLEMENT(FontRender)
