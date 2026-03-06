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
    return nullptr;
}

SGeometry* CVulkanResourceManager::CreateGeom(u32 FVF, ID3DVertexBuffer* vb, ID3DIndexBuffer* ib)
{
    return nullptr;
}

void CVulkanResourceManager::DeleteGeom(const SGeometry* VS)
{
}
