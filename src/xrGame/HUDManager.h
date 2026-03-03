#pragma once

#include "../xrEngine/CustomHUD.h"
#include "HitMarker.h"
#include "HUDTarget.h"
#include "GamePersistent.h"

class CHUDTarget;
class CUIGameCustom;

struct SPickParam
{
	collide::ray_defs defs;
	collide::rq_result result;
	float barrel_dist;
	bool barrel_blocked;
	Fmatrix barrel_matrix;
	float power;
	u32 pass;

	SPickParam(int cull) :
		defs(collide::ray_defs(Fvector(), Fvector(), 0.f, cull, collide::rqtBoth)),
		result(collide::rq_result().set(nullptr, 0.f, 0)),
		barrel_dist(0.f),
		barrel_blocked(false),
		barrel_matrix(Fmatrix().identity()),
		power(1.f),
		pass(0)
	{
	}

	SPickParam() : SPickParam(CDB::OPT_CULL) {}

	void InitPick()
	{
		defs.start.set(0, 0, 0);
		defs.dir.set(0, 0, 0);
		defs.range = g_pGamePersistent->Environment().CurrentEnv->far_plane;
		barrel_blocked = false;
		barrel_dist = 0.f;
		barrel_matrix.identity();
	}

	void CameraPick()
	{
		InitPick();
		defs.start = Device.vCameraPosition;
		defs.dir = Device.vCameraDirection;
		barrel_matrix = Device.mInvView;
	}
};

class CHUDManager :
	public CCustomHUD
{
	friend class CUI;
private:
	//.	CUI*					pUI;
	CUIGameCustom* pUIGame;
	CHitMarker HitMarker;
	CHUDTarget* m_pHUDTarget;
	bool b_online;
	SPickParam PP;
	collide::rq_results RQR;
public:
	CHUDManager();
	virtual ~CHUDManager() override;
	virtual void OnFrame() override;
	virtual void OnEvent(EVENT E, u64 P1, u64 P2) override;

	virtual void Render_First() override;
	virtual void Render_Last() override;

	virtual void RenderUI() override;

	//.				CUI*		GetUI				(){return pUI;}
	CUIGameCustom* GetGameUI()
	{
		return pUIGame;
	}

	void HitMarked(int idx, float power, const Fvector& dir);
	bool AddGrenade_ForMark(CGrenade* grn);
	void Update_GrenadeView(Fvector& pos_actor);
	void net_Relcase(CObject* obj) override;


	bool FireposActive();
	bool AimposActive();
	bool DoPick(SPickParam& pp);
	SPickParam& GetPick() { return PP; }
	collide::rq_result& GetRQ() { return GetPick().result; }

	//устанвка внешнего вида прицела в зависимости от текущей дисперсии
	void SetCrosshairDisp(float dispf, float disps = 0.f);
#ifdef DEBUG
    void					SetFirstBulletCrosshairDisp(float fbdispf);
#endif
	void ShowCrosshair(bool show);

	void SetHitmarkType(LPCSTR tex_name);
	void SetGrenadeMarkType(LPCSTR tex_name);

	virtual void OnScreenResolutionChanged() override;
	virtual void Load() override;
	virtual void OnDisconnected() override;
	virtual void OnConnected() override;

	virtual void RenderActiveItemUI() override;
	virtual void RenderCamAttachedUI() override;
	virtual bool RenderActiveItemUIQuery() override;
	virtual bool RenderCamAttachedUIQuery() override;

	//Lain: added
	void SetRenderable(bool renderable)
	{
		psHUD_Flags.set(HUD_DRAW_RT2, renderable);
	}

	virtual void Render_R1_Attachment_UI() override;
};

IC CHUDManager& HUD()
{
	return *((CHUDManager*)g_hud);
}
