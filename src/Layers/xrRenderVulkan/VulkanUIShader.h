#ifndef VulkanUIShader_included
#define VulkanUIShader_included
#pragma once

#include "../../Include/xrRender/UIShader.h"

class CVulkanUIShader : public IUIShader
{
public:
    CVulkanUIShader();
    virtual ~CVulkanUIShader();
    virtual void Copy(IUIShader& _in) override;
    virtual void create(LPCSTR sh, LPCSTR tex = 0) override;
    virtual bool inited() override;
    virtual void destroy() override;

    ref_shader& GetShader() { return hShader; }

private:
    ref_shader hShader;
};

#endif // VulkanUIShader_included
