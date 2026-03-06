#pragma once

#include "../xrRender/ResourceManager.h"

class CVulkanResourceManager : public CResourceManager
{
public:
    CVulkanResourceManager();
    virtual ~CVulkanResourceManager();

    virtual void OnDeviceCreate(LPCSTR name) override;
    virtual void OnDeviceDestroy(BOOL bKeepTextures) override;

    virtual CTexture* _CreateTexture(LPCSTR Name) override;
    virtual void _DeleteTexture(const CTexture* T) override;

    virtual SVS* _CreateVS(LPCSTR Name) override;
    virtual void _DeleteVS(const SVS* VS) override;

    virtual SPS* _CreatePS(LPCSTR Name) override;
    virtual void _DeletePS(const SPS* PS) override;

    virtual SGeometry* CreateGeom(D3DVERTEXELEMENT9* decl, ID3DVertexBuffer* vb, ID3DIndexBuffer* ib) override;
    virtual SGeometry* CreateGeom(u32 FVF, ID3DVertexBuffer* vb, ID3DIndexBuffer* ib) override;
    virtual void DeleteGeom(const SGeometry* VS) override;
};
