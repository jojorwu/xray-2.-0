#include "stdafx.h"
#pragma hdrstop

#include "../xrRender/ResourceManager.h"

#ifndef _EDITOR
#include "../../xrEngine/render.h"
#endif

#include "VulkanTexture.h"

void resptrcode_texture::create(LPCSTR _name)
{
    _set(DEV->_CreateTexture(_name));
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CTexture::CTexture()
{
    pSurface = nullptr;
    pAVI = nullptr;
    pTheora = nullptr;
    desc_cache = 0;
    seqMSPF = 0;
    flags.MemoryUsage = 0;
    flags.bLoaded = false;
    flags.bUser = false;
    flags.seqCycles = FALSE;
    m_material = 1.0f;
    bind = fastdelegate::FastDelegate1<u32>(this, &CTexture::apply_load);
}

CTexture::~CTexture()
{
    Unload();

    // release external reference
    DEV->_DeleteTexture(this);
}

void CTexture::surface_set(ID3DBaseTexture* surf)
{
    if (surf) surf->AddRef();

    if (pSurface) pSurface->Release();

    pSurface = surf;

    if (pSurface)
    {
        desc_update();
    }
}

ID3DBaseTexture* CTexture::surface_get()
{
    if (pSurface) pSurface->AddRef();
    return pSurface;
}

void CTexture::PostLoad()
{
    // For now only normal textures supported in Vulkan
    bind = fastdelegate::FastDelegate1<u32>(this, &CTexture::apply_normal);
}

void CTexture::apply_load(u32 dwStage)
{
    if (!flags.bLoaded) Load();
    else PostLoad();
    bind(dwStage);
}

void CTexture::apply_theora(u32 dwStage)
{
}

void CTexture::apply_avi(u32 dwStage)
{
}

void CTexture::apply_seq(u32 dwStage)
{
}

void CTexture::apply_normal(u32 dwStage)
{
    if (pSurface)
    {
        // Here we should bind the texture to the backend
        // VulkanBackend.SetTexture(dwStage, (CVulkanTexture*)pSurface);
    }
}

void CTexture::Preload()
{
    m_bumpmap = DEV->m_textures_description.GetBumpName(cName);
    m_material = DEV->m_textures_description.GetMaterial(cName);
}

void CTexture::Load()
{
    flags.bLoaded = true;
    desc_cache = 0;
    if (pSurface) return;

    flags.bUser = false;
    flags.MemoryUsage = 0;
    if (0 == stricmp(*cName, "$null")) return;
    if (0 != strstr(*cName, "$user$"))
    {
        flags.bUser = true;
        return;
    }

    Preload();

    // Normal texture
    u32 mem = 0;
    pSurface = ::Render->texture_load(*cName, mem);

    // Calc memory usage and preload into vid-mem
    if (pSurface)
    {
        flags.MemoryUsage = mem;
    }

    PostLoad();
}

void CTexture::Unload()
{
    flags.bLoaded = FALSE;
    if (!seqDATA.empty())
    {
        for (u32 I = 0; I < seqDATA.size(); I++)
        {
            if (seqDATA[I]) seqDATA[I]->Release();
        }
        seqDATA.clear();
        pSurface = 0;
    }
    flags.MemoryUsage = 0;

    if (pSurface) pSurface->Release();
    pSurface = nullptr;

    bind = fastdelegate::FastDelegate1<u32>(this, &CTexture::apply_load);
}

void CTexture::desc_update()
{
    desc_cache = pSurface;
    if (pSurface)
    {
        // TODO: Update desc from pSurface
        desc.Width = pSurface->GetWidth();
        desc.Height = pSurface->GetHeight();
    }
}

void CTexture::video_Play(BOOL looped, u32 _time)
{
}

void CTexture::video_Pause(BOOL state)
{
}

void CTexture::video_Stop()
{
}

BOOL CTexture::video_IsPlaying()
{
    return FALSE;
}
