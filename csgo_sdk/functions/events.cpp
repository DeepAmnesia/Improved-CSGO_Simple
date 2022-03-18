#include "../functions/events.hpp"
#include "../render/render.hpp"
#include "../render/menu.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <limits>
#include <ctime>

template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(Utils::PatternScan(GetModuleHandleA("client.dll"), "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto FindHudElement = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)FindHudElement(pThis, name);
}
void C_HookedEvents::DrawBeams()
{
	if (impacts.empty())
		return;

	while (!impacts.empty())
	{
		if (impacts.begin()->ImpactPosition == NULL) 
		{
			impacts.erase(impacts.begin());
			continue;
		}

		if (fabs(g_GlobalVars->curtime - impacts.begin()->Time) > 4.0f)
		{
			impacts.erase(impacts.begin());
			continue;
		}

		if (impacts.begin()->pEntity->IsDormant())
		{
			impacts.erase(impacts.begin());
			continue;
		}

		if (TIME_TO_TICKS(g_GlobalVars->curtime) > TIME_TO_TICKS(impacts.begin()->Time))
		{
			auto color = g_Configurations.enemy_bullet_tracer;

			if (impacts.begin()->pEntity == g_LocalPlayer)
			{
				if (!g_Configurations.use_local_bullet_tracers)
				{
					impacts.erase(impacts.begin());
					continue;
				}

				color = g_Configurations.local_bullet_tracer;
				//	color.a() = color.a() * (1 - m_globals()->m_curtime);
			}
			else if (!g_Configurations.use_enemy_bullet_tracers)
			{
				impacts.erase(impacts.begin());
				continue;
			}

	//		DrawBeam(impacts.begin()->pEntity == g_LocalPlayer, impacts.begin()->pEntity == g_LocalPlayer ? g_Configurations.g_Globals.last_shoot_position : impacts.begin()->pEntity->get_shoot_position(), impacts.begin()->ImpactPosition, color);
			impacts.erase(impacts.begin());
			continue;
		}

		break;
	}
}
void C_HookedEvents::DrawBeam(bool local_tracer, const Vector& src, const Vector& end, Color color)
{
	if (src == NULL)
		return;

	BeamInfo_t beam_info;
	beam_info.m_vecStart = src;

	if (local_tracer)
		beam_info.m_vecStart.z -= 2.0f;

	beam_info.m_vecEnd = end;
	beam_info.m_nType = TE_BEAMPOINTS;
	beam_info.m_pszModelName = "sprites/physbeam.vmt";
	beam_info.m_nModelIndex = -1;
	beam_info.m_flHaloScale = 0.0f;
	beam_info.m_flLife = 4.0f;
	beam_info.m_flWidth = 2.0f;
	beam_info.m_flEndWidth = 2.0f;
	beam_info.m_flFadeLength = 10.0f;
	beam_info.m_flAmplitude = 2.0f;
	beam_info.m_flBrightness = (float)color.a();
	beam_info.m_flSpeed = 0.2f;
	beam_info.m_nStartFrame = 0;
	beam_info.m_flFrameRate = 0.0f;
	beam_info.m_flRed = (float)color.r();
	beam_info.m_flGreen = (float)color.g();
	beam_info.m_flBlue = (float)color.b();
	beam_info.m_nSegments = 2;
	beam_info.m_bRenderable = true;
	beam_info.m_nFlags = FBEAM_SHADEIN | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

	auto beam = g_ViewRenderBeams->CreateBeamPoints(beam_info);

	if (beam)
		g_ViewRenderBeams->DrawBeam(beam);
}


void C_HookedEvents::FireGameEvent(IGameEvent* pEvent)
{
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
		return;

	if (!pEvent)
		return;

	if (!strcmp(pEvent->GetName(), "player_hurt"))
	{
		C_BasePlayer* hurt = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"))));
		C_BasePlayer* attacker = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker")));
		if (hurt != g_LocalPlayer && attacker == g_LocalPlayer)
		{
			if (g_Configurations.hitsound)
			{
				g_VGuiSurface->PlaySound_("buttons\\arena_switch_press_02.wav");
			}
			if (g_Configurations.damage_indicator)
			{

				int dmg_health = pEvent->GetInt("dmg_health");
				int hitgroup = pEvent->GetInt("hitgroup");
				DamageIndicator_t DmgIndicator;
				DmgIndicator.startTime = g_GlobalVars->curtime;
				DmgIndicator.hitgroup = hitgroup;
				DmgIndicator.hitPosition = damage_indicators.c_impactpos;
				DmgIndicator.damage = dmg_health;
				DmgIndicator.randomIdx = Math::RandomFloat(-20.f, 20.f);
				DmgIndicator.valid = true;
				if (damage_indicators.c_impactpos.x)
					damage_indicators.floatingTexts.push_back(DmgIndicator);
			}
		}
	}
	if (!strcmp(pEvent->GetName(), "player_footstep"))
	{
		if (g_Configurations.sound_esp)
		{
			auto userid = g_EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
			auto e = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(userid));

			if (g_Configurations.esp_enemies_only ? (e->m_iTeamNum() != g_LocalPlayer->m_iTeamNum()) && e != g_LocalPlayer : e != g_LocalPlayer)
			{
				switch (g_Configurations.sound_esp_type)
				{
				case 0:
					Render::Get().RenderCircle3D(e->GetAbsOrigin() + Vector(0.0f, 0.0f, 5.0f), 50, (float)g_Configurations.sound_esp_radius, g_Configurations.esp_sounds_color);
					break;
				case 1:
				{
					BeamInfo_t info;

					info.m_nType = TE_BEAMRINGPOINT;
					info.m_pszModelName = "sprites/physbeam.vmt";
					info.m_nModelIndex = g_MdlInfo->GetModelIndex("sprites/physbeam.vmt");
					info.m_nHaloIndex = -1;
					info.m_flHaloScale = 3.0f;
					info.m_flLife = 2.0f;
					info.m_flWidth = (float)3.f;
					info.m_flFadeLength = 1.0f;
					info.m_flAmplitude = 0.0f;
					info.m_flRed = (float)g_Configurations.esp_sounds_color.r();
					info.m_flGreen = (float)g_Configurations.esp_sounds_color.g();
					info.m_flBlue = (float)g_Configurations.esp_sounds_color.b();
					info.m_flBrightness = (float)g_Configurations.esp_sounds_color.a();
					info.m_flSpeed = 0.0f;
					info.m_nStartFrame = 0.0f;
					info.m_flFrameRate = 60.0f;
					info.m_nSegments = -1;
					info.m_nFlags = FBEAM_FADEOUT;
					info.m_vecCenter = e->GetAbsOrigin() + Vector(0.0f, 0.0f, 5.0f);
					info.m_flStartRadius = 5.0f;
					info.m_flEndRadius = (float)g_Configurations.sound_esp_radius;
					info.m_bRenderable = true;
					auto beam_draw = g_ViewRenderBeams->CreateBeamRingPoint(info);
					if (beam_draw)
						g_ViewRenderBeams->DrawBeam(beam_draw);
					break;
				}
				}
			}
		}
	}
	if (!strcmp(pEvent->GetName(), "bullet_impact"))
	{
		float x = pEvent->GetFloat("x");
		float y = pEvent->GetFloat("y");
		float z = pEvent->GetFloat("z");

		C_BasePlayer* target = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"))));

		if (!target || target != g_LocalPlayer)
			return;

		damage_indicators.c_impactpos = Vector(x, y, z);

		auto user_id = pEvent->GetInt("userid");
		auto user =  g_EngineClient->GetPlayerForUserID(user_id);

		auto e = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(user));

		if (!e->IsDormant())
		{
			if (e == g_LocalPlayer)
			{
				auto new_record = true;
				Vector position(x,y,z);

				for (auto& current : impacts)
				{
					if (e == current.pEntity)
					{
						new_record = false;

						current.ImpactPosition = position;
						current.Time = g_GlobalVars->curtime;
					}
				}

				if (new_record)
					impacts.push_back(
						ImpactData
						{
							e,
							position,
							g_GlobalVars->curtime
						});
			}
			else if (e->m_iTeamNum() != g_LocalPlayer->m_iTeamNum())
			{
				auto new_record = true;
				Vector position(x,y,z);

				for (auto& current : impacts)
				{
					if (e == current.pEntity)
					{
						new_record = false;

						current.ImpactPosition = position;
						current.Time = g_GlobalVars->curtime;
					}
				}

				if (new_record)
					impacts.push_back(
						ImpactData
						{
							e,
							position,
							g_GlobalVars->curtime
						});
			}
		}
	}

	if (!strcmp(pEvent->GetName(), "player_death"))
	{
		static DWORD* DeathNotice = FindHudElement<DWORD>("CCSGO_HudDeathNotice");
		static void(__thiscall * ClearNotices)(DWORD) = (void(__thiscall*)(DWORD))Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 EC 0C 53 56 8B 71 58");
		if (DeathNotice)
			*(float*)((DWORD)DeathNotice + 0x50) = g_Configurations.preserve_killfeed && g_LocalPlayer && g_LocalPlayer->IsAlive() ? FLT_MAX : 1.5;
	}
}

int C_HookedEvents::GetEventDebugID(void)
{
	return EVENT_DEBUG_ID_INIT;
}

void C_HookedEvents::RegisterSelf()
{
	m_iDebugId = EVENT_DEBUG_ID_INIT;
	g_GameEvents->AddListener(this, "player_footstep", false);
	g_GameEvents->AddListener(this, "player_hurt", false);
	g_GameEvents->AddListener(this, "player_death", false);
	g_GameEvents->AddListener(this, "weapon_fire", false);
	g_GameEvents->AddListener(this, "item_purchase", false);
	g_GameEvents->AddListener(this, "bullet_impact", false);
	g_GameEvents->AddListener(this, "round_start", false);
	g_GameEvents->AddListener(this, "round_freeze_end", false);
	g_GameEvents->AddListener(this, "bomb_defused", false);
	g_GameEvents->AddListener(this, "bomb_begindefuse", false);
	g_GameEvents->AddListener(this, "bomb_beginplant", false);
}

void C_HookedEvents::RemoveSelf()
{
	g_GameEvents->RemoveListener(this);
}