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
    if (0 == xr_strcmp(Name, "null")) return nullptr;

    auto it = m_textures.find(Name);
    if (it != m_textures.end()) return it->second;

    CTexture* T = xr_new<CTexture>();
    T->dwFlags |= xr_resource_flagged::RF_REGISTERED;
    m_textures.insert(std::make_pair(xr_strdup(Name), T));

    T->Preload();
    if (Device.b_is_Ready && !bDeferredLoad) T->Load();

    return T;
}

void CVulkanResourceManager::_DeleteTexture(const CTexture* T)
{
    for (auto it = m_textures.begin(); it != m_textures.end(); ++it)
    {
        if (it->second == T)
        {
            xr_free(it->first);
            m_textures.erase(it);
            xr_delete(T);
            return;
        }
    }
}

SVS* CVulkanResourceManager::_CreateVS(LPCSTR Name)
{
    auto it = m_vs.find(Name);
    if (it != m_vs.end())
        return it->second;

    SVS* VS = xr_new<SVS>();
    CVulkanShader shader;
    shader.Load(Name);
    VS->vs = shader.GetModule();
    // VS->constants = ...; // Need to handle reflection
    m_vs.insert(std::make_pair(xr_strdup(Name), VS));
    return VS;
}

void CVulkanResourceManager::_DeleteVS(const SVS* VS)
{
    for (auto it = m_vs.begin(); it != m_vs.end(); ++it)
    {
        if (it->second == VS)
        {
            if (VS->vs != VK_NULL_HANDLE)
                vkDestroyShaderModule(VulkanHW.GetDevice(), VS->vs, nullptr);

            xr_free(it->first);
            m_vs.erase(it);
            xr_delete(VS);
            return;
        }
    }
}

SPS* CVulkanResourceManager::_CreatePS(LPCSTR Name)
{
    auto it = m_ps.find(Name);
    if (it != m_ps.end())
        return it->second;

    SPS* PS = xr_new<SPS>();
    CVulkanShader shader;
    shader.Load(Name);
    PS->ps = shader.GetModule();
    // PS->constants = ...;
    m_ps.insert(std::make_pair(xr_strdup(Name), PS));
    return PS;
}

void CVulkanResourceManager::_DeletePS(const SPS* PS)
{
    for (auto it = m_ps.begin(); it != m_ps.end(); ++it)
    {
        if (it->second == PS)
        {
            if (PS->ps != VK_NULL_HANDLE)
                vkDestroyShaderModule(VulkanHW.GetDevice(), PS->ps, nullptr);

            xr_free(it->first);
            m_ps.erase(it);
            xr_delete(PS);
            return;
        }
    }
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
    ZeroMemory(dcl, sizeof(dcl));
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
    int count = 0;
    while (dcl[count].Stream != 0xFF) count++;
    count++; // Include end marker

    for (auto it : v_declarations)
    {
        if (it->dcl_code.size() != count) continue;
        if (memcmp(it->dcl_code.data(), dcl, count * sizeof(D3DVERTEXELEMENT9)) == 0)
            return it;
    }

    SDeclaration* D = xr_new<SDeclaration>();
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
