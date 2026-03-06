#include "stdafx.h"
#include "VulkanRenderFactory.h"
#include "VulkanRenderDeviceRender.h"

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
RENDER_FACTORY_IMPLEMENT(UIShader)
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

RENDER_FACTORY_IMPLEMENT(FlareRender)
RENDER_FACTORY_IMPLEMENT(ThunderboltRender)
RENDER_FACTORY_IMPLEMENT(ThunderboltDescRender)
RENDER_FACTORY_IMPLEMENT(RainRender)
RENDER_FACTORY_IMPLEMENT(LensFlareRender)
RENDER_FACTORY_IMPLEMENT(ImGuiRender)
RENDER_FACTORY_IMPLEMENT(EnvironmentRender)
RENDER_FACTORY_IMPLEMENT(EnvDescriptorMixerRender)
RENDER_FACTORY_IMPLEMENT(EnvDescriptorRender)
RENDER_FACTORY_IMPLEMENT(FontRender)
