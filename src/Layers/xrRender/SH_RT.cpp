#include "stdafx.h"
#pragma hdrstop

#include "ResourceManager.h"

#ifdef USE_VULKAN
#include "VulkanTexture.h"
#else
#include "dxRenderDeviceRender.h"
#endif

CRT::CRT()
{
	pSurface = nullptr;
	pRT = nullptr;
	dwWidth = 0;
	dwHeight = 0;
	fmt = D3DFMT_UNKNOWN;
}

CRT::~CRT()
{
	destroy();

	// release external reference
	DEV->_DeleteRT(this);
}

#ifdef USE_VULKAN
static VkFormat ConvertFormat(D3DFORMAT f)
{
    switch (f)
    {
    case D3DFMT_A8R8G8B8: return VK_FORMAT_B8G8R8A8_UNORM;
    case D3DFMT_A16B16G16R16F: return VK_FORMAT_R16G16B16A16_SFLOAT;
    case D3DFMT_R32F: return VK_FORMAT_R32_SFLOAT;
    case D3DFMT_D24S8: return VK_FORMAT_D24_UNORM_S8_UINT;
    case D3DFMT_D16: return VK_FORMAT_D16_UNORM;
    default: return VK_FORMAT_UNDEFINED;
    }
}

void CRT::create(LPCSTR Name, u32 w, u32 h, D3DFORMAT f, u32 SampleCount)
{
    if (pSurface) return;

    dwWidth = w;
    dwHeight = h;
    fmt = f;

    VkFormat vk_fmt = ConvertFormat(f);
    if (vk_fmt == VK_FORMAT_UNDEFINED)
    {
        Msg("! Vulkan: Unsupported RT format %d", f);
        return;
    }

    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (f == D3DFMT_D24S8 || f == D3DFMT_D16)
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    else
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    pSurface = xr_new<CVulkanTexture>();
    pSurface->Create(w, h, 1, vk_fmt, usage);
    pRT = pSurface->GetRTView();

    pTexture = DEV->_CreateTexture(Name);
    pTexture->surface_set(pSurface);
}

void CRT::destroy()
{
    if (pTexture._get())
    {
        pTexture->surface_set(0);
        pTexture.destroy();
        pTexture = nullptr;
    }

    if (pSurface)
    {
        xr_delete(pSurface);
        pSurface = nullptr;
        pRT = nullptr;
    }
}
#else
void CRT::create(LPCSTR Name, u32 w, u32 h, D3DFORMAT f, u32 SampleCount)
{
	if (pSurface) return;

	R_ASSERT(HW.pDevice && Name && Name[0] && w && h);
	_order = CPU::GetCLK(); //RDEVICE.GetTimerGlobal()->GetElapsed_clk();

	HRESULT _hr;

	dwWidth = w;
	dwHeight = h;
	fmt = f;

	// Get caps
	D3DCAPS9 caps;
	R_CHK(HW.pDevice->GetDeviceCaps(&caps));

	// Pow2
	if (!btwIsPow2(w) || !btwIsPow2(h))
	{
		if (!HW.Caps.raster.bNonPow2) return;
	}

	// Check width-and-height of render target surface
	if (w > caps.MaxTextureWidth) return;
	if (h > caps.MaxTextureHeight) return;

	// Select usage
	u32 usage = 0;
	if (D3DFMT_D24X8 == fmt) usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D24S8 == fmt) usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D15S1 == fmt) usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D16 == fmt) usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D16_LOCKABLE == fmt) usage = D3DUSAGE_DEPTHSTENCIL;
	else if ((D3DFORMAT)MAKEFOURCC('D', 'F', '2', '4') == fmt) usage = D3DUSAGE_DEPTHSTENCIL;
	else usage = D3DUSAGE_RENDERTARGET;

	// Validate render-target usage
	_hr = HW.pD3D->CheckDeviceFormat(
		HW.DevAdapter,
		HW.DevT,
		HW.Caps.fTarget,
		usage,
		D3DRTYPE_TEXTURE,
		f
	);
	if (FAILED(_hr)) return;

	// Try to create texture/surface
	DEV->Evict();
	_hr = HW.pDevice->CreateTexture(w, h, 1, usage, f, D3DPOOL_DEFAULT, &pSurface,nullptr);
	HW.stats_manager.increment_stats_rtarget(pSurface);

	if (FAILED(_hr) || (0 == pSurface)) return;

	// OK
#ifdef DEBUG
	Msg			("* created RT(%s), %dx%d",Name,w,h);
#endif // DEBUG
	R_CHK(pSurface->GetSurfaceLevel (0,&pRT));
	pTexture = DEV->_CreateTexture(Name);
	pTexture->surface_set(pSurface);
}

void CRT::destroy()
{
	if (pTexture._get())
	{
		pTexture->surface_set(0);
		pTexture.destroy();
		pTexture = nullptr;	 
	}

	_RELEASE(pRT);

	HW.stats_manager.decrement_stats_rtarget(pSurface);
	_RELEASE(pSurface);
}
#endif

void CRT::reset_begin()
{
	destroy();
}

void CRT::reset_end()
{
	create(*cName, dwWidth, dwHeight, fmt);
}

void resptrcode_crt::create(LPCSTR Name, u32 w, u32 h, D3DFORMAT f, u32 SampleCount)
{
	_set(DEV->_CreateRT(Name, w, h, f));
}


//////////////////////////////////////////////////////////////////////////
//	DX10 cut 
/*
CRTC::CRTC			()
{
	if (pSurface)	return;

	pSurface									= nullptr;
	pRT[0]=pRT[1]=pRT[2]=pRT[3]=pRT[4]=pRT[5]	= nullptr;
	dwSize										= 0;
	fmt											= D3DFMT_UNKNOWN;
}
CRTC::~CRTC			()
{
	destroy			();

	// release external reference
	DEV->_DeleteRTC	(this);
}

void CRTC::create	(LPCSTR Name, u32 size,	D3DFORMAT f)
{
	R_ASSERT	(HW.pDevice && Name && Name[0] && size && btwIsPow2(size));
	_order		= CPU::GetCLK();	//RDEVICE.GetTimerGlobal()->GetElapsed_clk();

	HRESULT		_hr;

	dwSize		= size;
	fmt			= f;

	// Get caps
	D3DCAPS9	caps;
	R_CHK		(HW.pDevice->GetDeviceCaps(&caps));

	// Check width-and-height of render target surface
	if (size>caps.MaxTextureWidth)		return;
	if (size>caps.MaxTextureHeight)		return;

	// Validate render-target usage
	_hr = HW.pD3D->CheckDeviceFormat(
		HW.DevAdapter,
		HW.DevT,
		HW.Caps.fTarget,
		D3DUSAGE_RENDERTARGET,
		D3DRTYPE_CUBETEXTURE,
		f
		);
	if (FAILED(_hr))					return;

	// Try to create texture/surface
	DEV->Evict					();
	_hr = HW.pDevice->CreateCubeTexture	(size, 1, D3DUSAGE_RENDERTARGET, f, D3DPOOL_DEFAULT, &pSurface,nullptr);
	if (FAILED(_hr) || (0==pSurface))	return;

	// OK
	Msg			("* created RTc(%s), 6(%d)",Name,size);
	for (u32 face=0; face<6; face++)
		R_CHK	(pSurface->GetCubeMapSurface	((D3DCUBEMAP_FACES)face, 0, pRT+face));
	pTexture	= DEV->_CreateTexture	(Name);
	pTexture->surface_set						(pSurface);
}

void CRTC::destroy		()
{
	pTexture->surface_set	(0);
	pTexture				= nullptr;
	for (u32 face=0; face<6; face++)
		_RELEASE	(pRT[face]	);
	_RELEASE	(pSurface	);
}
void CRTC::reset_begin	()
{
	destroy		();
}
void CRTC::reset_end	()
{
	create		(*cName,dwSize,fmt);
}

void resptrcode_crtc::create(LPCSTR Name, u32 size, D3DFORMAT f)
{
	_set		(DEV->_CreateRTC(Name,size,f));
}
*/
