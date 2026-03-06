#include "stdafx.h"
#include "VulkanResourceManager.h"
#include "VulkanTexture.h"

CVulkanResourceManager::CVulkanResourceManager()
{
}

CVulkanResourceManager::~CVulkanResourceManager()
{
}

void CVulkanResourceManager::OnDeviceCreate(LPCSTR name)
{
    // CResourceManager::OnDeviceCreate(name);
}

void CVulkanResourceManager::OnDeviceDestroy(BOOL bKeepTextures)
{
    // CResourceManager::OnDeviceDestroy(bKeepTextures);
}

CTexture* CVulkanResourceManager::_CreateTexture(LPCSTR Name)
{
    // Skeleton implementation
    return nullptr;
}

void CVulkanResourceManager::_DeleteTexture(const CTexture* T)
{
}

SVS* CVulkanResourceManager::_CreateVS(LPCSTR Name)
{
    return nullptr;
}

void CVulkanResourceManager::_DeleteVS(const SVS* VS)
{
}

SPS* CVulkanResourceManager::_CreatePS(LPCSTR Name)
{
    return nullptr;
}

void CVulkanResourceManager::_DeletePS(const SPS* PS)
{
}

SGeometry* CVulkanResourceManager::CreateGeom(D3DVERTEXELEMENT9* decl, ID3DVertexBuffer* vb, ID3DIndexBuffer* ib)
{
    SDeclaration* dcl = _CreateDecl(decl);

    for (auto it : v_geoms)
    {
        if (it->dcl == dcl && it->vb == vb && it->ib == ib)
            return it;
    }

    SGeometry* geom = xr_new<SGeometry>();
    geom->dcl = dcl;
    geom->vb = vb;
    geom->ib = ib;
    v_geoms.push_back(geom);
    return geom;
}

SGeometry* CVulkanResourceManager::CreateGeom(u32 FVF, ID3DVertexBuffer* vb, ID3DIndexBuffer* ib)
{
    D3DVERTEXELEMENT9 dcl[MAX_FVF_DECL_SIZE];
    // Need D3DX replacement for D3DXDeclaratorFromFVF on Linux/Vulkan
    // CHK_DX(D3DXDeclaratorFromFVF(FVF, dcl));
    return CreateGeom(dcl, vb, ib);
}

void CVulkanResourceManager::DeleteGeom(const SGeometry* VS)
{
    for (auto it = v_geoms.begin(); it != v_geoms.end(); ++it)
    {
        if (*it == VS)
        {
            v_geoms.erase(it);
            xr_delete(VS);
            return;
        }
    }
}

SDeclaration* CVulkanResourceManager::_CreateDecl(D3DVERTEXELEMENT9* dcl)
{
    for (auto it : v_declarations)
    {
        // Simple comparison for now
        if (memcmp(it->dcl_code.data(), dcl, it->dcl_code.size() * sizeof(D3DVERTEXELEMENT9)) == 0)
            return it;
    }

    SDeclaration* D = xr_new<SDeclaration>();
    int count = 0;
    while (dcl[count].Stream != 0xFF) count++;
    count++; // Include end marker
    D->dcl_code.assign(dcl, dcl + count);
    v_declarations.push_back(D);
    return D;
}

void CVulkanResourceManager::_DeleteDecl(const SDeclaration* dcl)
{
    for (auto it = v_declarations.begin(); it != v_declarations.end(); ++it)
    {
        if (*it == dcl)
        {
            v_declarations.erase(it);
            xr_delete(dcl);
            return;
        }
    }
}
