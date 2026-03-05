#pragma once

#include "../../xrEngine/Render.h"

class CVulkanRender : public IRender_interface
{
public:
    CVulkanRender();
    virtual ~CVulkanRender();

    // IRender_interface implementation
    virtual GenerationLevel get_generation() override { return GENERATION_R2; } // Using R2 as base for now
    virtual bool is_sun_static() override { return false; }
    virtual DWORD get_dx_level() override { return 0; } // Not DX

    virtual void create() override;
    virtual void destroy() override;

private:
    VkInstance m_instance;
    VkPhysicalDevice m_physical_device;
    VkDevice m_device;
    VkQueue m_graphics_queue;
    VkQueue m_present_queue;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapchain;
    xr_vector<VkImage> m_swapchain_images;
    virtual void reset_begin() override;
    virtual void reset_end() override;

    virtual void level_Load(IReader*) override;
    virtual void level_Unload() override;

    virtual HRESULT shader_compile(
        LPCSTR name,
        DWORD const* pSrcData,
        UINT SrcDataLen,
        LPCSTR pFunctionName,
        LPCSTR pTarget,
        DWORD Flags,
        void*& result) override;

    virtual LPCSTR getShaderPath() override { return "vulkan\\"; }
    virtual IRender_Sector* getSector(int id) override { return nullptr; }
    virtual IRenderVisual* getVisual(int id) override { return nullptr; }
    virtual IRender_Sector* detectSector(const Fvector& P) override { return nullptr; }
    virtual IRender_Target* getTarget() override { return nullptr; }

    virtual void set_Transform(Fmatrix* M) override {}
    virtual void set_HUD(BOOL V) override {}
    virtual BOOL get_HUD() override { return FALSE; }
    virtual void set_CamAttached(BOOL V) override {}
    virtual BOOL get_CamAttached() override { return FALSE; }
    virtual void set_Invisible(BOOL V) override {}
    virtual void flush() override {}
    virtual void set_Object(IRenderable* O) override {}
    virtual void add_Occluder(Fbox2& bb_screenspace) override {}
    virtual void add_Visual(IRenderVisual* V) override {}
    virtual void add_Geometry(IRenderVisual* V) override {}
    virtual void add_StaticWallmark(const wm_shader& S, const Fvector& P, float s, CDB::TRI* T, Fvector* V) override {}
    virtual void add_StaticWallmark(IWallMarkArray* pArray, const Fvector& P, float s, CDB::TRI* T, Fvector* V, float ttl = 0.f, bool ignore_opt = false, bool random_rotation = true) override {}
    virtual void add_StaticWallmark(IWallMarkArray* pArray, const Fvector& P, float s, CDB::TRI* T, Fvector* V, float ttl, bool ignore_opt, float rotation) override {}
    virtual void clear_static_wallmarks() override {}
    virtual void add_SkeletonWallmark(const Fmatrix* xf, IKinematics* obj, IWallMarkArray* pArray, const Fvector& start, const Fvector& dir, float size, float ttl = 0.f, bool ignore_opt = false) override {}

    virtual IRender_ObjectSpecific* ros_create(IRenderable* parent) override { return nullptr; }
    virtual void ros_destroy(IRender_ObjectSpecific*&) override {}

    virtual IRender_Light* light_create() override { return nullptr; }
    virtual IRender_Glow* glow_create() override { return nullptr; }

    virtual IRenderVisual* model_CreateParticles(LPCSTR name) override { return nullptr; }
    virtual IRenderVisual* model_Create(LPCSTR name, IReader* data = 0) override { return nullptr; }
    virtual IRenderVisual* model_CreateChild(LPCSTR name, IReader* data) override { return nullptr; }
    virtual IRenderVisual* model_Duplicate(IRenderVisual* V) override { return nullptr; }
    virtual void model_Delete(IRenderVisual*& V, BOOL bDiscard = FALSE) override {}
    virtual void model_Logging(BOOL bEnable) override {}
    virtual void models_Prefetch() override {}
    virtual void models_PrefetchOne(LPCSTR name, bool assert = true) override {}
    virtual void models_Clear(BOOL b_complete) override {}
    virtual bool models_Exists(LPCSTR name) override { return false; }

    virtual BOOL occ_visible(vis_data& V) override { return TRUE; }
    virtual BOOL occ_visible(Fbox& B) override { return TRUE; }
    virtual BOOL occ_visible(sPoly& P) override { return TRUE; }

    virtual void Calculate() override {}
    virtual void Render() override {}

    virtual void Screenshot(ScreenshotMode mode = SM_NORMAL, LPCSTR name = 0) override {}
    virtual void Screenshot(ScreenshotMode mode, CMemoryWriter& memory_writer) override {}
    virtual void ScreenshotAsyncBegin() override {}
    virtual void ScreenshotAsyncEnd(CMemoryWriter& memory_writer) override {}

    virtual void TakeScreenshot(LPCSTR path, Fvector2 dimensions, DxEncoding encoding = eDXE_A8R8G8B8) override {}

    virtual void rmNear() override {}
    virtual void rmFar() override {}
    virtual void rmNormal() override {}
    virtual u32 memory_usage() override { return 0; }
    virtual u32 active_phase() override { return 0; }
    virtual void RenderToTarget(RRT target) override {}

protected:
    virtual void ScreenshotImpl(ScreenshotMode mode, LPCSTR name, CMemoryWriter* memory_writer) override {}
};

extern CVulkanRender VulkanRenderImpl;
