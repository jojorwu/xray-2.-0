#ifndef	xrD3DDefs_included
#define	xrD3DDefs_included
#pragma once

#if defined(USE_DX11) || defined(USE_DX10)

#	include "..\xrRenderDX10\DXCommonTypes.h"

#elif defined(USE_VULKAN)

#include <vulkan/vulkan.h>

typedef VkShaderModule ID3DVertexShader;
typedef VkShaderModule ID3DPixelShader;
typedef void ID3DBlob;
typedef void D3D_SHADER_MACRO;
typedef void ID3DQuery;
typedef VkViewport D3D_VIEWPORT;
typedef void ID3DInclude;
typedef class CVulkanTexture ID3DTexture2D;
typedef struct CVulkanRTView ID3DRenderTargetView;
typedef struct CVulkanRTView ID3DDepthStencilView;
typedef class CVulkanTexture ID3DBaseTexture;
typedef VkImageCreateInfo D3D_TEXTURE2D_DESC;
typedef class CVulkanBuffer ID3DVertexBuffer;
typedef class CVulkanBuffer ID3DIndexBuffer;
typedef VkImage ID3DTexture3D;
typedef VkPipeline ID3DState;

#define DX10_ONLY(expr)			do {} while (0)

#else	//	USE_DX10

typedef IDirect3DVertexShader9 ID3DVertexShader;
typedef IDirect3DPixelShader9 ID3DPixelShader;
typedef ID3DXBuffer ID3DBlob;
typedef D3DXMACRO D3D_SHADER_MACRO;
typedef IDirect3DQuery9 ID3DQuery;
typedef D3DVIEWPORT9 D3D_VIEWPORT;
typedef ID3DXInclude ID3DInclude;
typedef IDirect3DTexture9 ID3DTexture2D;
typedef IDirect3DSurface9 ID3DRenderTargetView;
typedef IDirect3DSurface9 ID3DDepthStencilView;
typedef IDirect3DBaseTexture9 ID3DBaseTexture;
typedef D3DSURFACE_DESC D3D_TEXTURE2D_DESC;
typedef IDirect3DVertexBuffer9 ID3DVertexBuffer;
typedef IDirect3DIndexBuffer9 ID3DIndexBuffer;
typedef IDirect3DVolumeTexture9 ID3DTexture3D;
typedef IDirect3DStateBlock9 ID3DState;

#define DX10_ONLY(expr)			do {} while (0)

#endif	//	USE_DX10


#endif	//	xrD3DDefs_included
