#include "stdafx.h"
#include "VulkanUIShader.h"

CVulkanUIShader::CVulkanUIShader()
{
}

CVulkanUIShader::~CVulkanUIShader()
{
    destroy();
}

void CVulkanUIShader::Copy(IUIShader& _in)
{
    CVulkanUIShader& in = (CVulkanUIShader&)_in;
    hShader = in.hShader;
}

void CVulkanUIShader::create(LPCSTR sh, LPCSTR tex)
{
    hShader.create(sh, tex);
}

bool CVulkanUIShader::inited()
{
    return hShader;
}

void CVulkanUIShader::destroy()
{
    hShader.destroy();
}
