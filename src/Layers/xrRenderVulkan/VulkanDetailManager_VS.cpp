#include "stdafx.h"
#include "../xrRender/DetailManager.h"
#include "VulkanBackend.h"

void CDetailManager::hw_Render_dump(const Fvector4& consts, const Fvector4& wave, const Fvector4& wind,
									const Fvector4& prev_wave, const Fvector4& prev_wind, u32 var_id, u32 lod_id)
{
    // Implementation of grass/detail instancing for Vulkan
    // For now, reuse the DirectX 11 logic but adapted for Vulkan backend calls

    Device.Statistic->RenderDUMP_DT_Count = 0;

    u32 vOffset = 0;
    u32 iOffset = 0;

    vis_list& list = m_visibles[var_id];

    // Iterate through objects
    for (u32 O = 0; O < objects.size(); O++)
    {
        CDetail& Object = *objects[O];
        xr_vector<SlotItemVec*>& vis = list[O];
        if (!vis.empty())
        {
            // Set shader and pass
            RCache.set_Element(Object.shader->E[lod_id]);
            RImplementation.apply_lmaterial();

            // Set constants
            RCache.set_c("consts", consts);
            RCache.set_c("wave", wave);
            RCache.set_c("dir2D", wind);
            RCache.set_c("xform", Device.mFullTransform);

            u32 dwBatch = 0;

            // Temporary storage for instance data
            // In a better implementation, we'd use a dedicated instance buffer
            // For now, we'll use the constant table 'array' which maps to a uniform buffer

            struct InstanceData {
                Fvector4 m0, m1, m2, color;
            };
            InstanceData instance_buffer[64]; // hw_BatchSize is usually <= 64

            for (auto& items : vis)
            {
                for (auto& Instance : *items)
                {
                    u32 base = dwBatch;
                    float scale = Instance->scale_calculated;
                    Fmatrix& M = Instance->mRotY;

                    instance_buffer[base].m0.set(M._11 * scale, M._21 * scale, M._31 * scale, M._41);
                    instance_buffer[base].m1.set(M._12 * scale, M._22 * scale, M._32 * scale, M._42);
                    instance_buffer[base].m2.set(M._13 * scale, M._23 * scale, M._33 * scale, M._43);

                    float h = Instance->c_hemi;
                    float s = Instance->c_sun;
                    instance_buffer[base].color.set(s, s, s, h);

                    dwBatch++;
                    if (dwBatch == hw_BatchSize)
                    {
                        // Flush batch
                        RCache.set_ca("array", 0, &instance_buffer[0].m0, dwBatch * 4);

                        u32 dwCNT_verts = dwBatch * Object.number_vertices;
                        u32 dwCNT_prims = (dwBatch * Object.number_indices) / 3;

                        RCache.Render(D3DPT_TRIANGLELIST, vOffset, 0, dwCNT_verts, iOffset, dwCNT_prims);

                        Device.Statistic->RenderDUMP_DT_Count += dwBatch;
                        dwBatch = 0;
                    }
                }
            }

            if (dwBatch)
            {
                RCache.set_ca("array", 0, &instance_buffer[0].m0, dwBatch * 4);
                u32 dwCNT_verts = dwBatch * Object.number_vertices;
                u32 dwCNT_prims = (dwBatch * Object.number_indices) / 3;
                RCache.Render(D3DPT_TRIANGLELIST, vOffset, 0, dwCNT_verts, iOffset, dwCNT_prims);
                Device.Statistic->RenderDUMP_DT_Count += dwBatch;
            }

            vis.clear_not_free();
        }
        vOffset += hw_BatchSize * Object.number_vertices;
        iOffset += hw_BatchSize * Object.number_indices;
    }
}
