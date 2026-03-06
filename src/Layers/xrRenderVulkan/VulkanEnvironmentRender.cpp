#include "stdafx.h"
#include "VulkanEnvironmentRender.h"
#include "../../xrEngine/Environment.h"
#include "../../xrEngine/xr_efflensflare.h"

// half box def
static Fvector3 hbox_verts[24] =
{
    {-1.f, -1.f, -1.f}, {-1.f, -1.01f, -1.f}, // down
    {1.f, -1.f, -1.f}, {1.f, -1.01f, -1.f}, // down
    {-1.f, -1.f, 1.f}, {-1.f, -1.01f, 1.f}, // down
    {1.f, -1.f, 1.f}, {1.f, -1.01f, 1.f}, // down
    {-1.f, 1.f, -1.f}, {-1.f, 1.f, -1.f},
    {1.f, 1.f, -1.f}, {1.f, 1.f, -1.f},
    {-1.f, 1.f, 1.f}, {-1.f, 1.f, 1.f},
    {1.f, 1.f, 1.f}, {1.f, 1.f, 1.f},
    {-1.f, 0.f, -1.f}, {-1.f, -1.f, -1.f}, // half
    {1.f, 0.f, -1.f}, {1.f, -1.f, -1.f}, // half
    {1.f, 0.f, 1.f}, {1.f, -1.f, 1.f}, // half
    {-1.f, 0.f, 1.f}, {-1.f, -1.f, 1.f} // half
};
static u16 hbox_faces[20 * 3] =
{
    0, 2, 3,
    3, 1, 0,
    4, 5, 7,
    7, 6, 4,
    0, 1, 9,
    9, 8, 0,
    8, 9, 5,
    5, 4, 8,
    1, 3, 10,
    10, 9, 1,
    9, 10, 7,
    7, 5, 9,
    3, 2, 11,
    11, 10, 3,
    10, 11, 6,
    6, 7, 10,
    2, 0, 8,
    8, 11, 2,
    11, 8, 4,
    4, 6, 11
};

#pragma pack(push,1)
struct v_skybox
{
    Fvector3 p;
    u32 color;
    Fvector3 uv[2];

    void set(Fvector3& _p, u32 _c, Fvector3& _tc)
    {
        p = _p;
        color = _c;
        uv[0] = _tc;
        uv[1] = _tc;
    }
};

struct v_clouds
{
    Fvector3 p;
    u32 color;
    u32 intensity;

    void set(Fvector3& _p, u32 _c, u32 _i)
    {
        p = _p;
        color = _c;
        intensity = _i;
    }
};
#pragma pack(pop)

// CVulkanEnvDescriptorRender
CVulkanEnvDescriptorRender::CVulkanEnvDescriptorRender()
{
}

CVulkanEnvDescriptorRender::~CVulkanEnvDescriptorRender()
{
}

void CVulkanEnvDescriptorRender::Copy(IEnvDescriptorRender& _in)
{
    CVulkanEnvDescriptorRender& in = (CVulkanEnvDescriptorRender&)_in;
    sky_r_textures = in.sky_r_textures;
    sky_r_textures_env = in.sky_r_textures_env;
    clouds_r_textures = in.clouds_r_textures;
}

void CVulkanEnvDescriptorRender::OnDeviceCreate(CEnvDescriptor& owner)
{
    if (owner.sky_texture_name.size())
        sky_r_textures.create(owner.sky_texture_name.c_str());

    if (owner.sky_texture_env_name.size())
        sky_r_textures_env.create(owner.sky_texture_env_name.c_str());

    if (owner.clouds_texture_name.size())
        clouds_r_textures.create(owner.clouds_texture_name.c_str());
}

void CVulkanEnvDescriptorRender::OnDeviceDestroy()
{
    sky_r_textures.destroy();
    sky_r_textures_env.destroy();
    clouds_r_textures.destroy();
}

// CVulkanEnvDescriptorMixerRender
CVulkanEnvDescriptorMixerRender::CVulkanEnvDescriptorMixerRender()
{
}

CVulkanEnvDescriptorMixerRender::~CVulkanEnvDescriptorMixerRender()
{
}

void CVulkanEnvDescriptorMixerRender::Copy(IEnvDescriptorMixerRender& _in)
{
    CVulkanEnvDescriptorMixerRender& in = (CVulkanEnvDescriptorMixerRender&)_in;
    sky_r_textures = in.sky_r_textures;
    sky_r_textures_env = in.sky_r_textures_env;
    clouds_r_textures = in.clouds_r_textures;
}

void CVulkanEnvDescriptorMixerRender::Destroy()
{
    sky_r_textures.destroy();
    sky_r_textures_env.destroy();
    clouds_r_textures.destroy();
}

void CVulkanEnvDescriptorMixerRender::Clear()
{
    sky_r_textures.destroy();
    sky_r_textures_env.destroy();
    clouds_r_textures.destroy();
}

void CVulkanEnvDescriptorMixerRender::lerp(IEnvDescriptorRender* inA, IEnvDescriptorRender* inB)
{
    CVulkanEnvDescriptorRender* A = (CVulkanEnvDescriptorRender*)inA;
    CVulkanEnvDescriptorRender* B = (CVulkanEnvDescriptorRender*)inB;

    sky_r_textures = A->sky_r_textures;
    sky_r_textures_env = A->sky_r_textures_env;
    clouds_r_textures = A->clouds_r_textures;
}

// CVulkanEnvironmentRender
CVulkanEnvironmentRender::CVulkanEnvironmentRender()
{
}

CVulkanEnvironmentRender::~CVulkanEnvironmentRender()
{
}

void CVulkanEnvironmentRender::Copy(IEnvironmentRender& _in)
{
}

void CVulkanEnvironmentRender::OnFrame(CEnvironment& env)
{
}

void CVulkanEnvironmentRender::OnLoad()
{
}

void CVulkanEnvironmentRender::OnUnload()
{
}

void CVulkanEnvironmentRender::RenderSky(CEnvironment& env, bool only_MV)
{
    if (env.bNeed_re_create_env)
    {
        OnDeviceCreate();
        env.bNeed_re_create_env = FALSE;
    }
    ::Render->rmFar();

    CVulkanEnvDescriptorMixerRender* mixRen = (CVulkanEnvDescriptorMixerRender*)env.CurrentEnv->m_pDescriptorMixer->m_pDescriptorMixerRender;

    // draw sky box
    Fmatrix mSky;
    mSky.rotateY(env.CurrentEnv->sky_rotation);
    mSky.translate_over(Device.vCameraPosition);

    u32 i_offset, v_offset;
    u32 C = color_rgba(iFloor(env.CurrentEnv->sky_color.x * 255.f), iFloor(env.CurrentEnv->sky_color.y * 255.f),
        iFloor(env.CurrentEnv->sky_color.z * 255.f), iFloor(env.CurrentEnv->weight * 255.f));

    // Fill index buffer
    u16* pib = RCache.Index.Lock(20 * 3, i_offset);
    CopyMemory(pib, hbox_faces, 20 * 3 * 2);
    RCache.Index.Unlock(20 * 3);

    // Fill vertex buffer
    v_skybox* pv = (v_skybox*)RCache.Vertex.Lock(12, sh_2geom.stride(), v_offset);
    for (u32 v = 0; v < 12; v++) pv[v].set(hbox_verts[v * 2], C, hbox_verts[v * 2 + 1]);
    RCache.Vertex.Unlock(12, sh_2geom.stride());

    // Apply skybox matrix
    RCache.set_xform_world(mSky);

    RCache.set_Geometry(sh_2geom);
    RCache.set_Shader(sh_2sky);
    // RCache.set_Textures(&mixRen->sky_r_textures);
    RCache.Render(D3DPT_TRIANGLELIST, v_offset, 0, 12, i_offset, 20);

    // Sun
    ::Render->rmNormal();

    if (!only_MV)
    {
        RCache.set_Z(FALSE);
        RCache.set_Z(TRUE);
        env.eff_LensFlare->Render(TRUE, FALSE, FALSE);
        RCache.set_Z(FALSE);
    }
}

void CVulkanEnvironmentRender::RenderClouds(CEnvironment& env)
{
    ::Render->rmFar();

    Fmatrix mXFORM, mScale;
    mScale.scale(10, 0.4f, 10);
    mXFORM.rotateY(env.CurrentEnv->sky_rotation);
    mXFORM.mulB_43(mScale);
    mXFORM.translate_over(Device.vCameraPosition);

    Fvector wd0, wd1;
    Fvector4 wind_dir;
    wd0.setHP(PI_DIV_4, 0);
    wd1.setHP(PI_DIV_4 + PI_DIV_8, 0);
    wind_dir.set(wd0.x, wd0.z, wd1.x, wd1.z).mul(0.5f).add(0.5f).mul(255.f);
    u32 i_offset, v_offset;
    u32 C0 = color_rgba(iFloor(wind_dir.x), iFloor(wind_dir.y), iFloor(wind_dir.w), iFloor(wind_dir.z));
    u32 C1 = color_rgba(iFloor(env.CurrentEnv->clouds_color.x * 255.f), iFloor(env.CurrentEnv->clouds_color.y * 255.f),
        iFloor(env.CurrentEnv->clouds_color.z * 255.f), iFloor(env.CurrentEnv->clouds_color.w * 255.f));

    // Fill index buffer
    u16* pib = RCache.Index.Lock(env.CloudsIndices.size(), i_offset);
    CopyMemory(pib, &env.CloudsIndices.front(), env.CloudsIndices.size() * sizeof(u16));
    RCache.Index.Unlock(env.CloudsIndices.size());

    // Fill vertex buffer
    v_clouds* pv = (v_clouds*)RCache.Vertex.Lock(env.CloudsVerts.size(), clouds_geom.stride(), v_offset);
    for (FvectorIt it = env.CloudsVerts.begin(); it != env.CloudsVerts.end(); it++, pv++)
        pv->set(*it, C0, C1);
    RCache.Vertex.Unlock(env.CloudsVerts.size(), clouds_geom.stride());

    // Render
    RCache.set_xform_world(mXFORM);
    RCache.set_Geometry(clouds_geom);
    RCache.set_Shader(clouds_sh);
    // CVulkanEnvDescriptorMixerRender* mixRen = (CVulkanEnvDescriptorMixerRender*)env.CurrentEnv->m_pDescriptorMixer->m_pDescriptorMixerRender;
    // RCache.set_Textures(&mixRen->clouds_r_textures);
    RCache.Render(D3DPT_TRIANGLELIST, v_offset, 0, env.CloudsVerts.size(), i_offset, env.CloudsIndices.size() / 3);

    ::Render->rmNormal();
}

void CVulkanEnvironmentRender::OnDeviceCreate()
{
    const u32 v_skybox_fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE3(1);
    const u32 v_clouds_fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR;

    sh_2sky.create("sky2", "skybox\\sky_morning");
    sh_2geom.create(v_skybox_fvf, RCache.Vertex.Buffer(), RCache.Index.Buffer());

    clouds_sh.create("clouds", "skybox\\clouds");
    clouds_geom.create(v_clouds_fvf, RCache.Vertex.Buffer(), RCache.Index.Buffer());
}

void CVulkanEnvironmentRender::OnDeviceDestroy()
{
    sh_2sky.destroy();
    sh_2geom.destroy();
    clouds_sh.destroy();
    clouds_geom.destroy();
}

particles_systems::library_interface const& CVulkanEnvironmentRender::particles_systems_library()
{
    static particles_systems::library_interface* lib = nullptr;
    return *lib;
}
