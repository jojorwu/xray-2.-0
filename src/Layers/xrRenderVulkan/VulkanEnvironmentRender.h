#ifndef VulkanEnvironmentRender_included
#define VulkanEnvironmentRender_included
#pragma once

#include "../../Include/xrRender/EnvironmentRender.h"

class CVulkanEnvDescriptorRender : public IEnvDescriptorRender
{
public:
    CVulkanEnvDescriptorRender();
    virtual ~CVulkanEnvDescriptorRender();
    virtual void Copy(IEnvDescriptorRender& _in) override;

    virtual void OnDeviceCreate(CEnvDescriptor& owner) override;
    virtual void OnDeviceDestroy() override;

    ref_texture sky_r_textures;
    ref_texture sky_r_textures_env;
    ref_texture clouds_r_textures;
};

class CVulkanEnvDescriptorMixerRender : public IEnvDescriptorMixerRender
{
public:
    CVulkanEnvDescriptorMixerRender();
    virtual ~CVulkanEnvDescriptorMixerRender();
    virtual void Copy(IEnvDescriptorMixerRender& _in) override;

    virtual void Destroy() override;
    virtual void Clear() override;
    virtual void lerp(IEnvDescriptorRender* inA, IEnvDescriptorRender* inB) override;

    ref_texture sky_r_textures;
    ref_texture sky_r_textures_env;
    ref_texture clouds_r_textures;
};

class CVulkanEnvironmentRender : public IEnvironmentRender
{
public:
    CVulkanEnvironmentRender();
    virtual ~CVulkanEnvironmentRender();
    virtual void Copy(IEnvironmentRender& _in) override;
    virtual void OnFrame(CEnvironment& env) override;
    virtual void OnLoad() override;
    virtual void OnUnload() override;
    virtual void RenderSky(CEnvironment& env, bool only_MV = false) override;
    virtual void RenderClouds(CEnvironment& env) override;
    virtual void OnDeviceCreate() override;
    virtual void OnDeviceDestroy() override;
    virtual particles_systems::library_interface const& particles_systems_library() override;

private:
    ref_geom sh_2geom;
    ref_shader sh_2sky;

    ref_geom clouds_geom;
    ref_shader clouds_sh;
};

#endif // VulkanEnvironmentRender_included
